/*
 * pt.c
 */

#include <lib.h>
#include <pt.h>

#define PT_LIST_END     (pt_list_entry_t *)4
static pt_list_entry_t  *list = PT_LIST_END;

void
pt_list_register(pt_list_entry_t *entry)
{
    if (entry->next == NULL) {
        entry->next = list;
        list = entry;
    }
}

void
pt_list_run(void)
{
    pt_list_entry_t     *entry = list;
    
    while (entry != PT_LIST_END) {
        entry->func(&entry->pt);
        entry = entry->next;
    }
}
