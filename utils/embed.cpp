
/*
 *
 * A Simple file embeder made in cpp for nekohook
 *
 */

 #include <stdlib.h> // c lib
 #include <stdio.h> // reading/writing to files
 #include <string.h> // string utils kill me

 int main(int argC, const char** argS) {
   // check args
   if (argC < 2) {
     printf("Usage: %s <File to embed> <Optional Header output>\n", argS[0]);
     return 1;
   }
   // Ensure we have an out
   const char* out_file_path;
   if (argS[2])
     out_file_path = argS[2];
   else {
     // Deleting this isnt needed, it will be destroyed on program end
     const char header_append[] = ".h";
     char* tmp = new char[strlen(argS[1]) + sizeof(header_append) + 1];
     strcpy(tmp, argS[1]);
     strcat(tmp, header_append);
     out_file_path = tmp;
   } 

   // Open the files we need, check both for errors
   FILE* in_file = fopen(argS[1], "r");
   if (!in_file) {
     printf("Cannot open file: \"%s\"\n", argS[1]);
     return 1;
   }
   FILE* out_file = fopen(out_file_path, "w");
   if (!out_file) {
     printf("Cannot open file for writing: \"%s\"\n", out_file_path);
     return 1;
   }
   
   // Print header preamble
   fprintf(out_file,
           "/*\n"
           " *\n"
           " * This file is auto-generated for nekohook!\n"
           " *\n"
           " * Original file: %s\n"
           " *\n"
           " */\n"
           "\n", argS[1]);
           
    // Start the embed array
    char* in_path = strcpy(new char[strlen(argS[1]) + 1], argS[1]);
    while (char* period = strstr(in_path, "."))
      *period = '_';
    fprintf(out_file, "const unsigned char %s[] = {\n\t", in_path);
    
    // Start reading the file and print to the header
    unsigned char buf[256];
    int tick = 1;
    while (size_t i = fread(buf, sizeof(char), sizeof(buf), in_file)) {
      for(size_t ii = 0; ii < i; ii++) {
        fprintf(out_file, "0x%02x, ", buf[ii]);
		if (tick++ >= 10) {
		  fputs("\n\t", out_file);
		  tick = 1;
		}
	  }
    }
    
    // Finish up
    if (tick) 
      fputs("\n", out_file);
    fputs("};\n", out_file);

    return 0;
 }
 