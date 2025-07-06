// CatalogDefs.h
// Common Catalog defines

#ifndef CATALOG_DEFS_H
#define CATALOG_DEFS_H

// Common Catalog Character array sizes
// These are 1 or 2 larger than actual to guarantee the null terminator is there
#define OBJNAME_LENGTH 19
#define CONS_LENGTH 4 
#define BAYER_LENGTH 7 
#define MAG_LENGTH 6
#define SUBID_LENGTH 7
#define OBJTYPE_LENGTH 15
#define RA_LENGTH 10
#define DEC_LENGTH 11

// Common Catalog defines
#define SD_CARD_LINE_LENGTH        110
#define NUM_CATALOG_ROWS_PER_SCREEN 14
#define MAX_CUSTOM_CATALOG_PAGES    10
#define MAX_CUSTOM_CATALOG_ROWS (MAX_CUSTOM_CATALOG_PAGES * NUM_CATALOG_ROWS_PER_SCREEN)

#endif