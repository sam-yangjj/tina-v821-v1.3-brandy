/*
 * Copyright (c) 2019-2025 Allwinner Technology Co., Ltd. ALL rights reserved.
 *
 * Allwinner is a trademark of Allwinner Technology Co.,Ltd., registered in
 * the people's Republic of China and other countries.
 * All Allwinner Technology Co.,Ltd. trademarks are used with permission.
 *
 * DISCLAIMER
 * THIRD PARTY LICENCES MAY BE REQUIRED TO IMPLEMENT THE SOLUTION/PRODUCT.
 * IF YOU NEED TO INTEGRATE THIRD PARTY’S TECHNOLOGY (SONY, DTS, DOLBY, AVS OR MPEGLA, ETC.)
 * IN ALLWINNERS’SDK OR PRODUCTS, YOU SHALL BE SOLELY RESPONSIBLE TO OBTAIN
 * ALL APPROPRIATELY REQUIRED THIRD PARTY LICENCES.
 * ALLWINNER SHALL HAVE NO WARRANTY, INDEMNITY OR OTHER OBLIGATIONS WITH RESPECT TO MATTERS
 * COVERED UNDER ANY REQUIRED THIRD PARTY LICENSE.
 * YOU ARE SOLELY RESPONSIBLE FOR YOUR USAGE OF THIRD PARTY’S TECHNOLOGY.
 *
 *
 * THIS SOFTWARE IS PROVIDED BY ALLWINNER"AS IS" AND TO THE MAXIMUM EXTENT
 * PERMITTED BY LAW, ALLWINNER EXPRESSLY DISCLAIMS ALL WARRANTIES OF ANY KIND,
 * WHETHER EXPRESS, IMPLIED OR STATUTORY, INCLUDING WITHOUT LIMITATION REGARDING
 * THE TITLE, NON-INFRINGEMENT, ACCURACY, CONDITION, COMPLETENESS, PERFORMANCE
 * OR MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE.
 * IN NO EVENT SHALL ALLWINNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL,
 * SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT
 * NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
 * LOSS OF USE, DATA, OR PROFITS, OR BUSINESS INTERRUPTION)
 * HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT,
 * STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
 * ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED
 * OF THE POSSIBILITY OF SUCH DAMAGE.
 */

/*
 * (C) Copyright 2007-2013
 * Allwinner Technology Co., Ltd. <www.allwinnertech.com>
 * Jerry Wang <wangflord@allwinnertech.com>
 * wangwei <wangwei@allwinnertech.com>
 */

#include <common.h>

struct alloc_struct_t
{
	phys_addr_t address;                      //申请内存的地址
    __u32 size;                         //分配的内存大小，用户实际得到的内存大小
    __u32 o_size;                       //用户申请的内存大小
    struct alloc_struct_t *next;
};

#ifdef CFG_RISCV_CACHE_LINE
#define MY_BYTE_ALIGN(x)                                                       \
	(((x + (CFG_RISCV_CACHE_LINE - 1)) / CFG_RISCV_CACHE_LINE) *           \
	 CFG_RISCV_CACHE_LINE)
#else
#define MY_BYTE_ALIGN(x) (((x + 15) / 16) * 16) /* alloc based on 1k byte */
#endif

static struct alloc_struct_t boot_heap_head, boot_heap_tail;
/*
*********************************************************************************************************
*                       CREATE HEAP
*
* Description: create heap.
*
* Aguments   : pHeapHead    heap start address.
*              nHeapSize    heap size.
*
* Returns    : EPDK_OK/EPDK_FAIL.
*********************************************************************************************************
*/

__s32 malloc_init(__u32 pHeapHead, __u32 nHeapSize)
{
    boot_heap_head.size    = boot_heap_tail.size = 0;
    boot_heap_head.address = pHeapHead;
    boot_heap_tail.address = pHeapHead + nHeapSize;
    boot_heap_head.next    = &boot_heap_tail;
    boot_heap_tail.next    = 0;

    return 0;
}

/*
*********************************************************************************************************
*                       MALLOC BUFFER FROM HEAP
*
* Description: malloc a buffer from heap.
*
* Aguments   : num_bytes    the size of the buffer need malloc;
*
* Returns    : the po__s32er to buffer has malloc.
*********************************************************************************************************
*/
void *malloc(__u32 num_bytes)
{
    struct alloc_struct_t *ptr, *newptr;
    __u32  actual_bytes;

    if (!num_bytes) return 0;

    actual_bytes = MY_BYTE_ALIGN(num_bytes);    /* translate the byte count to size of long type       */

    ptr = &boot_heap_head;                      /* scan from the boot_heap_head of the heap            */

    while (ptr && ptr->next)                    /* look for enough memory for alloc                    */
    {
        if (ptr->next->address >= (ptr->address + ptr->size +                                          \
                2 * sizeof(struct alloc_struct_t) + actual_bytes))
        {
            break;
        }
                                                /* find enough memory to alloc                         */
        ptr = ptr->next;
    }

    if (!ptr->next)
    {
        return 0;                   /* it has reached the boot_heap_tail of the heap now              */
    }

    newptr = (struct alloc_struct_t *)(ptr->address + ptr->size);
                                                /* create a new node for the memory block             */
    if (!newptr)
    {
        return 0;                               /* create the node failed, can't manage the block     */
    }

    /* set the memory block chain, insert the node to the chain */
    newptr->address = ptr->address + ptr->size + sizeof(struct alloc_struct_t);
#ifdef CFG_RISCV_CACHE_LINE
	newptr->address = MY_BYTE_ALIGN(newptr->address);
#endif
    newptr->size    = actual_bytes;
    newptr->o_size  = num_bytes;
    newptr->next    = ptr->next;
    ptr->next       = newptr;

    return (void *)newptr->address;
}
/*
*********************************************************************************************************
*                       MALLOC BUFFER FROM HEAP
*
* Description: malloc a buffer from heap.
*
* Aguments   : num_bytes    the size of the buffer need malloc;
*
* Returns    : the po__s32er to buffer has malloc.
*********************************************************************************************************
*/
void *realloc(void *p, __u32 num_bytes)
{
    struct alloc_struct_t *ptr, *prev;
    void   *tmp;
    __u32  actual_bytes;

    if(!p)
    {
        return malloc(num_bytes);             /* 如果传进的指针是空，则按照传进的字节数申请内存    */
    }
    if (!num_bytes)
    {
        return p;                                   /* 如果申请追加的内存字节数是0，则直接返回当前的指针 */
    }

    ptr = &boot_heap_head;                          /* look for the node which po__s32 this memory block                   */
    while (ptr && ptr->next)
    {
		if (ptr->next->address == (phys_addr_t)p)
            break;                                  /* find the node which need to be release                              */
        ptr = ptr->next;
    }

    //此时，ptr指向的是用户指针的前一个节点
    prev = ptr;
    ptr  = ptr->next;

    if(!ptr)
    {
        return 0;                                   /* 如果没有找到用户传进的地址         */
    }
    //用ptr指向用户节点
    actual_bytes = MY_BYTE_ALIGN(ptr->o_size + num_bytes);
    if(actual_bytes == ptr->size)
    {
        return p;
    }

    if (ptr->next->address >= (ptr->address + actual_bytes +                                          \
                2 * sizeof(struct alloc_struct_t)))
    {
        ptr->size    = actual_bytes;
        ptr->o_size += num_bytes;

        return p;
    }

    tmp = malloc(actual_bytes);
    if(!tmp)
    {
        return 0;
    }
    memcpy(tmp, (void *)ptr->address, ptr->size);
    prev->next = ptr->next;     /* delete the node which need be released from the memory block chain  */

    return tmp;
}

/*
*********************************************************************************************************
*                       FREE BUFFER TO HEAP
*
* Description: free buffer to heap
*
* Aguments   : p    the po__s32er to the buffer which need be free.
*
* Returns    : none
*********************************************************************************************************
*/
void  free(void *p)
{
    struct alloc_struct_t *ptr, *prev;

	if( p == NULL )
		return;

    ptr = &boot_heap_head;                /* look for the node which po__s32 this memory block                     */
    while (ptr && ptr->next)
    {
		if (ptr->next->address == (phys_addr_t)p)
            break;              /* find the node which need to be release                              */
        ptr = ptr->next;
    }

	prev = ptr;
	ptr = ptr->next;

    if (!ptr) return;           /* the node is heap boot_heap_tail                                               */

    prev->next = ptr->next;     /* delete the node which need be released from the memory block chain  */

    return ;
}






