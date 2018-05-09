#ifndef CHAIN_ELEMENT_
#define CHAIN_ELEMENT_
#include "chain_of_responsibility.h"

/*! @struct chain_element
 * @brief ChainElement class instance definition, detail is in chain_element.c
*/
struct chain_element;/*! extend DPUtilListData defined in design_pattern_util/include/dp_list.h */
typedef struct chain_element chain_element_t, *ChainElement;

ChainElement chain_element_new(void);
int chain_element_add_function(ChainElement this, chain_func func);
void chain_element_remove_function(ChainElement this, chain_func func);
void chain_element_call(ChainElement this, void *arg);
void chain_element_delete(ChainElement this);
#endif
