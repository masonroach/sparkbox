#ifndef SPARKBOX_EXT_MEM
#define SPARKBOX_EXT_MEM

// Define the external memory start and end locations
// Based on STM32F407 FSMC Bank 3
#define EXTERNAL_MEM_BASE 				0x8000000U
#define EXTERNAL_MEM_END 				0X8FFFFFFU
#define EXTERNAL_MEM_SIZE_BYTES			0x1000000U

// Initialize the external memory interface on a hardware level
void external_init(void);

// Allocates memory for data in external memory
void *external_malloc(uint32_t sizeBytes);

// Frees memory previously allocated
void external_free(uint32_t sizeBytes);

#endif
