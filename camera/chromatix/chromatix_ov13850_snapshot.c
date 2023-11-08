/*============================================================================


  Copyright (c) 2013 Qualcomm Technologies, Inc. All Rights Reserved.
  Qualcomm Technologies Proprietary and Confidential.


============================================================================*/


/*============================================================================
 *                      INCLUDE FILES
 *===========================================================================*/
#include "chromatix.h"
//#include "sensor_dbg.h"


static chromatix_parms_type chromatix_ov13850_parms = {
#include "chromatix_ov13850_snapshot.h"
};


/*============================================================================
 * FUNCTION    - load_chromatix -
 *
 * DESCRIPTION:
 *==========================================================================*/
void *load_chromatix(void)
{
//  SLOW("chromatix ptr %p", &chromatix_ov13850_parms);
//  SERR("jinhui, load chromatix_ov13850_snapshot.h");
  return &chromatix_ov13850_parms;
}
