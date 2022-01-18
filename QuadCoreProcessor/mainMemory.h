#ifndef __MAIN_MEMORY_H__
#define __MAIN_MEMORY_H__

#include <stdint.h>


void MainMemory_Init(void);

/*!
******************************************************************************
\brief
 Print memory data to file.
\details
 Called only at the end
\return none
*****************************************************************************/
void MainMemory_PrintData(void);

#endif //__MAIN_MEMORY_H__
