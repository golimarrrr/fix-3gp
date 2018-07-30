#define _GNU_SOURCE
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

int main (int argc, char *argv[]) {
  FILE * pFileBroken;
  FILE * pFileOk;
  FILE * pFileResult;
  long lSizeBroken;
  long lSizeOk;
  char * bufferFileBroken;
  char * bufferFileOk;
  size_t result;    
  char * mdatPosBroken;
  char * mdatPosOk;
  int * pMdatSectionSize;
  int mdatSectionSize;
  long newMdatSectionSize;
  size_t maxFileSize;


  if (argc < 3) {
    fprintf(
      stderr,
      "Usage: %s brokenFile.3gp okFile.3gp\nTries to fix 3GP file using another file with the same format (i.e. recorded with the same application) and same or longer length\n",
      argv[0]
      );
    exit(4);
  }

  maxFileSize = 400*1024*1024;    // 400 MiB

  //// BROKEN FILE

  printf("Opening BROKEN file: %s\n" , argv[1]);
  pFileBroken = fopen (argv[1] , "rb");
  if (pFileBroken == NULL) {
    fputs("File error\n",stderr);
    exit(1);
  }

  // obtain file size:
  fseek (pFileBroken, 0, SEEK_END);
  lSizeBroken = ftell(pFileBroken);
  printf("File size: %d\n" , lSizeBroken);
  rewind (pFileBroken);

  if (lSizeBroken > maxFileSize) {
    fprintf(stderr, "Max file size to load into memory: %d\n", maxFileSize);
    exit(5);
  }

  // allocate memory to contain the whole file:
  bufferFileBroken = (char*) malloc (sizeof(char)*lSizeBroken);
  if (bufferFileBroken == NULL) {
    fputs ("Memory error\n",stderr);
    exit (2);
  }

  // copy the file into the buffer:
  result = fread(bufferFileBroken, 1, lSizeBroken, pFileBroken);
  if (result != lSizeBroken) {
    fputs("Reading error\n",stderr);
    exit (3);
  }

  mdatPosBroken = memmem(bufferFileBroken, lSizeBroken, "mdat", 4);
  printf("mdat found at: %d (0x%x)\n", mdatPosBroken-bufferFileBroken, mdatPosBroken-bufferFileBroken);

  pMdatSectionSize = (int *) (mdatPosBroken-4);
  printf("mdat section size declared by 3gp file: %d (0x%x), integer value size: %d\n", *pMdatSectionSize, *pMdatSectionSize, sizeof(*pMdatSectionSize));

  if (*pMdatSectionSize > lSizeBroken) {
    fprintf(stderr, "Invalid declared section size, corrupted file (will use real size and not declared size)\n");
  }

  newMdatSectionSize = lSizeBroken-(mdatPosBroken-bufferFileBroken);     // we will write from mdat section start until end of file
  printf("mdat section real size: %d (0x%x)\n", newMdatSectionSize, newMdatSectionSize);

  fclose (pFileBroken);



  //// OK FILE

  printf("Opening OK file: %s\n" , argv[2]);
  pFileOk = fopen (argv[2] , "rb");
  if (pFileOk == NULL) {
    fputs ("File error\n",stderr);
    exit (1);
  }

  // obtain file size:
  fseek (pFileOk, 0, SEEK_END);
  lSizeOk = ftell (pFileOk);
  printf("File size: %d\n" , lSizeOk);
  rewind (pFileOk);

  if (lSizeOk > maxFileSize) {
    fprintf(stderr, "Max file size to load into memory: %d\n", maxFileSize);
    exit(5);
  }

  // allocate memory to contain the whole file:
  bufferFileOk = (char*) malloc (sizeof(char)*lSizeOk);
  if (bufferFileOk == NULL) {
    fputs("Memory error\n",stderr);
    exit (2);
  }

  // copy the file into the bufferFileOk:
  result = fread(bufferFileOk, 1, lSizeOk, pFileOk);
  if (result != lSizeOk) {
    fputs ("Reading error\n",stderr);
    exit (3);
  }

  mdatPosOk = memmem(bufferFileOk, lSizeOk, "mdat", 4);
  printf("mdat found at: %d (0x%x)\n", mdatPosOk-bufferFileOk, mdatPosOk-bufferFileOk);

  pMdatSectionSize = (int *) (mdatPosOk-4);     // we point 4 bytes back, to the 4-byte big endian integer size value (3GP files are big endian)
  mdatSectionSize = __bswap_32(*pMdatSectionSize);  // big endian to little endian
  printf("mdat section size declared by 3gp file: %d (0x%x), integer value size: %d\n", mdatSectionSize, mdatSectionSize, sizeof(mdatSectionSize));
  if (mdatSectionSize > lSizeOk) {
    fprintf(stderr, "Invalid declared section size, corrupted file %d %d\n", mdatSectionSize, lSizeOk);
  }

  fclose (pFileOk);


  //// RESULT FILE
  printf("Writing result file\n");
  pFileResult = fopen ("result.3gp", "wb");
  if (pFileResult == NULL) {
    fputs("File error\n", stderr);
    exit (1);
  }

  // write from beginning of OK file until mdat section, without the 4-byte integer size value
  fwrite(bufferFileOk, (mdatPosOk-bufferFileOk)-4, 1, pFileResult);
  // write the new 4-byte integer size value
  int newMdatSectionSizeBigEndian = __bswap_32(newMdatSectionSize);
  fwrite(&newMdatSectionSizeBigEndian, 4, 1, pFileResult);
  // write the broken file mdat section
  fwrite(mdatPosBroken, newMdatSectionSize, 1, pFileResult);
  // write from end of OK file mdat section, to OK file end
  fwrite(mdatPosOk+mdatSectionSize, lSizeOk-mdatSectionSize, 1, pFileResult);

  printf("Finished, freeing up resources\n");

  // terminate
  free (bufferFileBroken);
  free (bufferFileOk);
  fclose(pFileResult);


  return 0;

}

