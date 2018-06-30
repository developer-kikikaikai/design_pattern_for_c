/**
 * @file prototype_factory.h
 * @brief This is API for PrototypeFactoryInstance class
**/
#ifndef PROTOTYPE_FACTORY_H
#define PROTOTYPE_FACTORY_H
#include "prototype.h"

/*! prototype_factory_instance, PrototypeFactory class instance definition*/
struct prototype_factory_instance_t;
typedef struct prototype_factory_instance_t prototype_factory_instance_t, *PrototypeFactoryInstance;

/*@name prototype_factory_instance API definition*/
/* @{ */
/*! new */
PrototypeFactoryInstance prototype_factory_instance_new(void * base, size_t base_length, prototype_factory_method_t * factory_method);
/*! free */
void prototype_factory_instance_free(PrototypeFactoryInstance this);
/*! clone data */
void * prototype_factory_instance_clone_data(PrototypeFactoryInstance this);
/*! free data */
void prototype_factory_instance_free_data(PrototypeFactoryInstance this, void * data);
/* @} */
#endif
