#include <net/if.h>
#include <mm/heap.h>
#include <lib/assert.h>
#include <lib/module.h>
#include <lib/string.h>

MODULE("interface-manager");

static net_interface_t* eth_if_buf = NULL;
static size_t eth_interface_count = 0;

static uint8_t get_if_values(const char* idstr, net_interface_t** if_buf, size_t** if_count) {
  switch (*idstr) {
    case 'e':
      if (kstrcmp(idstr, "eth") == 0) {
        *if_buf = eth_if_buf;
        *if_count = &eth_interface_count;
      }
      return 1;
    default:
      return 1;
  }

  return 0;
}

static net_interface_t* find_with_idstr(const char* idstr) {
  switch (*idstr) {
    case 'e':
      if (kstrcmp(idstr, "eth") == 0) {
        return &eth_if_buf[eth_interface_count - 1];
      }
  }

  return NULL;
}


void if_manager_init(void) {
  eth_if_buf = kmalloc(sizeof(net_interface_t));
  ASSERT(eth_if_buf != NULL, "if_list == NULL\n");
}


uint16_t create_interface(const char* idstr) {
  net_interface_t* if_list = NULL;
  size_t* if_list_size_ptr = NULL;

  if (get_if_values(idstr, &if_list, &if_list_size_ptr) != 0) {
    return 0;
  }

  net_interface_t* target = find_with_idstr(idstr);
  if (target == NULL) {
    // This interface doesn't exist, create new one.
    if_list[*if_list_size_ptr].idstr = idstr;
    if_list[*if_list_size_ptr++].id = 0;
    if_list = krealloc(if_list, sizeof(net_interface_t) * (*if_list_size_ptr + 1));
  } else {
    if_list[*if_list_size_ptr].idstr = idstr;
    if_list[*if_list_size_ptr++].id = target->id + 1;
    if_list = krealloc(if_list, sizeof(net_interface_t) * (*if_list_size_ptr + 1));
  }

  return if_list[*if_list_size_ptr - 1].id;
}


uint8_t interface_exists(const char* idstr, uint16_t id) {
  net_interface_t* if_list = NULL;
  size_t* if_list_size_ptr = NULL;
  
  if (get_if_values(idstr, &if_list, &if_list_size_ptr) != 0) {
    return 0;
  }

  for (size_t i = 0; i < *if_list_size_ptr; ++i) {
    if (kstrcmp(if_list[i].idstr, idstr) == 0 && if_list[i].id == id) {
      return 1;
    }
  }

  return 0;
}
