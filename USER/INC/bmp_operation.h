#ifndef BMP_OPERATION_H
#define BMP_OPERATION_H
#ifdef __cplusplus
 extern "C" {
#endif

#include "QSharedPointer"
#include "WaterMeter/rawimagetype.h"

QSharedPointer<RawImageCombination> img_rorate(QSharedPointer<RawImageCombination> imgIn,quadrange_t *quad);


#ifdef __cplusplus
 }
#endif

#endif // BMP_OPERATION_H
