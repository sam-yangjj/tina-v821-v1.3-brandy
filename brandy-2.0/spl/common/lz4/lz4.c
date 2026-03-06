/*
   LZ4 - Fast LZ compression algorithm
   Copyright (C) 2011-2015, Yann Collet.

   SPDX-License-Identifier: BSD-2-Clause

   You can contact the author at :
   - LZ4 source repository : https://github.com/Cyan4973/lz4
   - LZ4 public forum : https://groups.google.com/forum/#!forum/lz4c
*/

/**************************************
*  Reading and writing into memory
**************************************/

/* customized version of memcpy, which may overwrite up to 7 bytes beyond dstEnd */
static void LZ4_wildCopy(void *dstPtr, const void *srcPtr, void *dstEnd)
{
	BYTE *d       = (BYTE *)dstPtr;
	const BYTE *s = (const BYTE *)srcPtr;
	BYTE *e       = (BYTE *)dstEnd;
	do {
		LZ4_copy4(d, s);
		d += 4;
		s += 4;
	} while (d < e);
}

/**************************************
*  Common Constants
**************************************/
#define MINMATCH 4

#define COPYLENGTH 8
#define LASTLITERALS 5
#define MFLIMIT (COPYLENGTH + MINMATCH)
static const int LZ4_minLength = (MFLIMIT + 1);

#define KB *(1 << 10)
#define MB *(1 << 20)
#define GB *(1U << 30)

#define MAXD_LOG 16
#define MAX_DISTANCE ((1 << MAXD_LOG) - 1)

#define ML_BITS 4
#define ML_MASK ((1U << ML_BITS) - 1)
#define RUN_BITS (8 - ML_BITS)
#define RUN_MASK ((1U << RUN_BITS) - 1)
#ifdef min
#undef min
#endif
#define min(x, y)                                                              \
	({                                                                     \
		typeof(x) _min1 = (x);                                         \
		typeof(y) _min2 = (y);                                         \
		_min1 < _min2 ? _min1 : _min2;                                 \
	})

/**************************************
*  Local Structures and types
**************************************/
typedef enum { noDict = 0, withPrefix64k, usingExtDict } dict_directive;
typedef enum { endOnOutputSize = 0, endOnInputSize = 1 } endCondition_directive;
typedef enum { full = 0, partial = 1 } earlyEnd_directive;

/*******************************
*  Decompression functions
*******************************/
/*
 * This generic decompression function cover all use cases.
 * It shall be instantiated several times, using different sets of directives
 * Note that it is essential this generic function is really inlined,
 * in order to remove useless branches during compilation optimization.
 */
FORCE_INLINE int LZ4_decompress_generic(
	const char *const source, char *const dest, int inputSize,
	int outputSize, /* If endOnInput==endOnInputSize, this value is the max size of Output Buffer. */

	int endOnInput, /* endOnOutputSize, endOnInputSize */
	int partialDecoding, /* full, partial */
	int targetOutputSize, /* only used if partialDecoding==partial */
	int dict, /* noDict, withPrefix64k, usingExtDict */
	const BYTE *const lowPrefix, /* == dest if dict == noDict */
	const BYTE *const dictStart, /* only if dict==usingExtDict */
	const size_t dictSize /* note : = 0 if noDict */
	)
{
	/* Local Variables */
	const BYTE *ip	 = (const BYTE *)source;
	const BYTE *const iend = ip + inputSize;

	BYTE *op	 = (BYTE *)dest;
	BYTE *const oend = op + outputSize;
	BYTE *cpy;
	BYTE *oexit		   = op + targetOutputSize;
	const BYTE *const lowLimit = lowPrefix - dictSize;

	const BYTE *const dictEnd = (const BYTE *)dictStart + dictSize;
	const size_t dec32table[] = { 4, 1, 2, 1, 4, 4, 4, 4 };
	const size_t dec64table[] = { 0, 0, 0, (size_t)-1, 0, 1, 2, 3 };

	const int safeDecode  = (endOnInput == endOnInputSize);
	const int checkOffset = ((safeDecode) && (dictSize < (int)(64 KB)));

	/* Special cases */
	if ((partialDecoding) && (oexit > oend - MFLIMIT))
		oexit = oend -
			MFLIMIT; /* targetOutputSize too high => decode everything */
	if ((endOnInput) && (unlikely(outputSize == 0)))
		return ((inputSize == 1) && (*ip == 0)) ?
			       0 :
			       -1; /* Empty output buffer */
	if ((!endOnInput) && (unlikely(outputSize == 0)))
		return (*ip == 0 ? 1 : -1);

	/* Main Loop */
	while (1) {
		unsigned token;
		size_t length;
		const BYTE *match;

		/* get literal length */
		token = *ip++;

		length = (token >> ML_BITS);
		if (length == RUN_MASK) {
			unsigned s;
			do {
				s = *ip++;
				length += s;
			} while (likely((endOnInput) ? ip < iend - RUN_MASK :
						       1) &&
				 (s == 255));
			if ((safeDecode) && unlikely((op + length) < (op)))
				goto _output_error; /* overflow detection */
			if ((safeDecode) && unlikely((ip + length) < (ip)))
				goto _output_error; /* overflow detection */
		}

		/* copy literals */
		cpy = op + length;
		if (((endOnInput) &&
		     ((cpy > (partialDecoding ? oexit : oend - MFLIMIT)) ||
		      (ip + length > iend - (2 + 1 + LASTLITERALS)))) ||
		    ((!endOnInput) && (cpy > oend - COPYLENGTH))) {
			if (partialDecoding) {
				if (cpy > oend)
					goto _output_error; /* Error : write attempt beyond end of output buffer */
				if ((endOnInput) && (ip + length > iend))
					goto _output_error; /* Error : read attempt beyond end of input buffer */
			} else {
				if ((!endOnInput) && (cpy != oend))
					goto _output_error; /* Error : block decoding must stop exactly there */
				if ((endOnInput) &&
				    ((ip + length != iend) || (cpy > oend)))
					goto _output_error; /* Error : input must be consumed */
			}
			memcpy(op, ip, length);
			ip += length;
			op += length;
			break; /* Necessarily EOF, due to parsing restrictions */
		}
		LZ4_wildCopy(op, ip, cpy);
		ip += length;
		op = cpy;

		/* get offset */
		match = cpy - LZ4_readLE16(ip);
		ip += 2;
		if ((checkOffset) && (unlikely(match < lowLimit)))
			goto _output_error; /* Error : offset outside destination buffer */

		/* get matchlength */
		length = token & ML_MASK;
		if (length == ML_MASK) {
			unsigned s;
			do {
				if ((endOnInput) && (ip > iend - LASTLITERALS))
					goto _output_error;
				s = *ip++;
				length += s;
			} while (s == 255);
			if ((safeDecode) && unlikely((op + length) < op))
				goto _output_error; /* overflow detection */
		}
		length += MINMATCH;

		/* check external dictionary */
		if ((dict == usingExtDict) && (match < lowPrefix)) {
			if (unlikely(op + length > oend - LASTLITERALS))
				goto _output_error; /* doesn't respect parsing restriction */

			if (length <= (size_t)(lowPrefix - match)) {
				/* match can be copied as a single segment from external dictionary */
				match = dictEnd - (lowPrefix - match);
				memmove(op, match, length);
				op += length;
			} else {
				/* match encompass external dictionary and current segment */
				size_t copySize = (size_t)(lowPrefix - match);
				memcpy(op, dictEnd - copySize, copySize);
				op += copySize;
				copySize = length - copySize;
				/* overlap within current segment */
				if (copySize > (size_t)(op - lowPrefix)) {
					BYTE *const endOfMatch = op + copySize;
					const BYTE *copyFrom   = lowPrefix;
					while (op < endOfMatch)
						*op++ = *copyFrom++;
				} else {
					memcpy(op, lowPrefix, copySize);
					op += copySize;
				}
			}
			continue;
		}

		/* copy repeated sequence */
		cpy = op + length;
		if (unlikely((op - match) < 8)) {
			const size_t dec64 = dec64table[op - match];
			op[0]		   = match[0];
			op[1]		   = match[1];
			op[2]		   = match[2];
			op[3]		   = match[3];
			match += dec32table[op - match];
			LZ4_copy4(op + 4, match);
			op += 8;
			match -= dec64;
		} else {
			LZ4_copy4(op, match);
			op += 4;
			match += 4;
		}

		if (unlikely(cpy > oend - 12)) {
			if (cpy > oend - LASTLITERALS)
				goto _output_error; /* Error : last LASTLITERALS bytes must be literals */
			if (op < oend - 8) {
				LZ4_wildCopy(op, match, oend - 8);
				match += (oend - 8) - op;
				op = oend - 8;
			}
			while (op < cpy)
				*op++ = *match++;
		} else
			LZ4_wildCopy(op, match, cpy);
		op = cpy; /* correction */
	}

	/* end of decoding */
	if (endOnInput)
		return (int)(((char *)op) -
			     dest); /* Nb of output bytes decoded */
	else
		return (int)(((const char *)ip) -
			     source); /* Nb of input bytes read */

/* Overflow error detected */
_output_error:
	return (int)(-(((const char *)ip) - source)) - 1;
}

#if 1
FORCE_INLINE int LZ4_part_decompress_init(
	struct LZ4_part_decompress_ctrl *head,
	const char *const source, char *const dest, int inputSize,
	int outputSize, /* If endOnInput==endOnInputSize, this value is the max size of Output Buffer. */

	int endOnInput, /* endOnOutputSize, endOnInputSize */
	int partialDecoding, /* full, partial */
	int targetOutputSize, /* only used if partialDecoding==partial */
	int dict, /* noDict, withPrefix64k, usingExtDict */
	const BYTE *const lowPrefix, /* == dest if dict == noDict */
	const BYTE *const dictStart, /* only if dict==usingExtDict */
	const size_t dictSize /* note : = 0 if noDict */
	)
{
	/* Local Variables */
	BYTE *op	 = (BYTE *)dest;
	BYTE *const oend = op + outputSize;
	BYTE *oexit = op + targetOutputSize;

	if (unlikely(outputSize == 0))
		return -EINVAL; /* Empty output buffer */

	/* Special cases */
	if ((partialDecoding) && (oexit > oend - MFLIMIT))
		oexit = oend - MFLIMIT; /* targetOutputSize too high => decode everything */

	memset(head, 0, sizeof(*head));

	head->source = (const uint8_t *)source;
	head->dest = (uint8_t *)dest;
	head->inputSize = inputSize;
	head->outputSize = outputSize;
	head->endOnInput = endOnInput;
	head->partialDecoding = partialDecoding;
	head->targetOutputSize = targetOutputSize;
	head->dict = dict;
	head->lowPrefix = (uint8_t *)lowPrefix;
	head->dictStart = (uint8_t *)dictStart;
	head->dictSize = dictSize;
	head->oexit = oexit;
	head->state = LZ4_PART_PARSE_TOKEN;

	head->ip = (const uint8_t *)source;
	head->op = (uint8_t *)dest;
	head->safeDecode = (endOnInput == endOnInputSize);
	head->checkOffset = ((head->safeDecode) && (dictSize < (int)(64 KB)));

	return 0;
}

FORCE_INLINE int LZ4_part_decompress_is_complete(struct LZ4_part_decompress_ctrl *head)
{
	return head->state == LZ4_PART_FINISH;
}

FORCE_INLINE uint32_t LZ4_part_decompress_get_processed_cnt(struct LZ4_part_decompress_ctrl *head)
{
	return (uint32_t)(head->ip - head->source);
}

FORCE_INLINE int LZ4_part_decompress_seq(struct LZ4_part_decompress_ctrl *head, size_t size)
{
	BYTE *oexit = head->oexit;
	const BYTE *const iend = head->source + head->inputSize;
	BYTE *const oend = head->dest + head->outputSize;
	const BYTE *const lowLimit = head->lowPrefix - head->dictSize;

	const size_t dec32table[] = { 4, 1, 2, 1, 4, 4, 4, 4 };
	const size_t dec64table[] = { 0, 0, 0, (size_t)-1, 0, 1, 2, 3 };
	const BYTE *const dictEnd = (const BYTE *)head->dictStart + head->dictSize;

	const int safeDecode  = head->safeDecode;
	const int checkOffset = head->checkOffset;
	const int endOnInput = head->endOnInput;
	const int dict = head->dict;
	const int partialDecoding = head->partialDecoding;

	const uint8_t *ip_end;
	const uint8_t *match;
	const uint8_t *orig_ip;
	size_t length;
	const uint8_t *ip;
	uint8_t *op;
	unsigned char token;

	BYTE *cpy;

	if (head->state == LZ4_PART_FINISH)
		return 0;

	ip = head->ip;
	op = head->op;
	head->in_pos += size;
	ip_end = head->source + head->in_pos;

	if (head->state == LZ4_PART_PARSE_MATCH) {
		token = head->token;
		goto parse_match;
	}

	while (1) {
		if ((ip + MFLIMIT) > ip_end) {
			head->ip = ip;
			head->op = op;
			head->state = LZ4_PART_PARSE_TOKEN;
			return 0; /* no more data */
		}

		orig_ip = ip;

		/* get literal length */
		token = *ip++;

		length = (token >> ML_BITS);
		if (unlikely(length == RUN_MASK)) {
			unsigned s;
			do {
				s = *ip++;
				length += s;
			} while (likely((endOnInput) ? ip < iend - RUN_MASK : 1) && (s == 255));

			if ((safeDecode) && unlikely((op + length) < (op)))
				goto _output_error; /* overflow detection */
			if ((safeDecode) && unlikely((ip + length) < (ip)))
				goto _output_error; /* overflow detection */
		}

		if (unlikely((ip + length) >= ip_end)) {
			head->ip = orig_ip;
			head->op = op;
			head->state = LZ4_PART_PARSE_TOKEN;
			return 0; /* no more data */
		}

		/* copy literals */
		cpy = op + length;

		if (((endOnInput) &&
		     ((cpy > (partialDecoding ? oexit : oend - MFLIMIT)) ||
		      (ip + length > iend - (2 + 1 + LASTLITERALS)))) ||
		    ((!endOnInput) && (cpy > oend - COPYLENGTH))) {
			if (partialDecoding) {
				if (cpy > oend)
					goto _output_error; /* Error : write attempt beyond end of output buffer */
				if ((endOnInput) && (ip + length > iend))
					goto _output_error; /* Error : read attempt beyond end of input buffer */
			} else {
				if ((!endOnInput) && (cpy != oend))
					goto _output_error; /* Error : block decoding must stop exactly there */
				if ((endOnInput) &&
				    ((ip + length != iend) || (cpy > oend)))
					goto _output_error; /* Error : input must be consumed */
			}

			memcpy(op, ip, length);
			ip += length;
			op += length;

			head->ip = ip;
			head->op = op;
			head->state = LZ4_PART_FINISH;
			goto out; /* Necessarily EOF, due to parsing restrictions */
		}

		LZ4_wildCopy(op, ip, cpy);
		ip += length;
		op = cpy;
parse_match:
		orig_ip = ip;

		if ((ip + 2) >= ip_end) {
			head->ip = ip;
			head->op = op;
			head->token = token;
			head->state = LZ4_PART_PARSE_MATCH;
			return 0; /* no more data */
		}

		cpy = op;
		/* get offset */
		match = cpy - LZ4_readLE16(ip);
		ip += 2;
		if ((checkOffset) && (unlikely(match < lowLimit)))
			goto _output_error; /* Error : offset outside destination buffer */

		/* get matchlength */
		length = token & ML_MASK;
		if (unlikely(length == ML_MASK)) {
			unsigned s;
			do {
				if ((endOnInput) && (ip > iend - LASTLITERALS))
					goto _output_error;
				s = *ip++;
				length += s;

			} while (s == 255);
			if ((safeDecode) && unlikely((op + length) < op))
				goto _output_error; /* overflow detection */

			if (ip >= ip_end) {
				head->ip = orig_ip;
				head->op = op;
				head->token = token;
				head->state = LZ4_PART_PARSE_MATCH;
				return 0; /* no more data */
			}
		}
		length += MINMATCH;
		head->ip = ip;

		/* check external dictionary */
		if ((dict == usingExtDict) && (match < head->lowPrefix)) {
			if (unlikely(op + length > oend - LASTLITERALS))
				goto _output_error; /* doesn't respect parsing restriction */

			if (length <= (size_t)(head->lowPrefix - match)) {
				/* match can be copied as a single segment from external dictionary */
				match = dictEnd - (head->lowPrefix - match);
				memmove(op, match, length);
				op += length;
			} else {
				/* match encompass external dictionary and current segment */
				size_t copySize = (size_t)(head->lowPrefix - match);
				memcpy(op, dictEnd - copySize, copySize);
				op += copySize;
				copySize = length - copySize;
				/* overlap within current segment */
				if (copySize > (size_t)(op - head->lowPrefix)) {
					BYTE *const endOfMatch = op + copySize;
					const BYTE *copyFrom   = head->lowPrefix;
					while (op < endOfMatch)
						*op++ = *copyFrom++;
				} else {
					memcpy(op, head->lowPrefix, copySize);
					op += copySize;
				}
			}
			continue;
		}

		/* copy repeated sequence */
		cpy = op + length;
		if (unlikely((op - match) < 8)) {
			const size_t dec64 = dec64table[op - match];
			op[0]		   = match[0];
			op[1]		   = match[1];
			op[2]		   = match[2];
			op[3]		   = match[3];
			match += dec32table[op - match];
			LZ4_copy4(op + 4, match);
			op += 8;
			match -= dec64;
		} else {
			LZ4_copy4(op, match);
			op += 4;
			match += 4;
		}

		if (unlikely(cpy > oend - 12)) {
			if (cpy > oend - LASTLITERALS)
				goto _output_error; /* Error : last LASTLITERALS bytes must be literals */
			if (op < oend - 8) {
				LZ4_wildCopy(op, match, oend - 8);
				match += (oend - 8) - op;
				op = oend - 8;
			}
			while (op < cpy)
				*op++ = *match++;
		} else
			LZ4_wildCopy(op, match, cpy);
		op = cpy; /* correction */
		continue;
	}

out:
	/* end of decoding */
	if (endOnInput)
		return (int)(op - head->dest); /* Nb of output bytes decoded */
	else
		return (int)(ip - head->source); /* Nb of input bytes read */

/* Overflow error detected */
_output_error:
	printf("Overflow error detected\n");
	return (int)(-(ip - head->source)) - 1;
}
#endif
