/* psbook.c
 * (c) Reuben Thomas 2012-2016
 * (c) Angus J. C. Duggan 1991-1997
 * See file LICENSE for details.
 *
 * rearrange pages in conforming PS file for printing in signatures
 */

#include "config.h"

#include <unistd.h>
#include "progname.h"

#include "psutil.h"

const char *syntax = "[-q] [-sSIGNATURE] [INFILE [OUTFILE]]\n       SIGNATURE must be positive and divisible by 4\n";

const char *argerr_message = "";

int
main(int argc, char *argv[])
{
   int signature = 0;
   int currentpg, maxpage;
   int opt;

   set_program_name (argv[0]);

   verbose = 1;

   while((opt = getopt(argc, argv, "vqs:")) != EOF) {
     switch(opt) {
     case 's':	/* signature size */
       signature = atoi(optarg);
       if (signature < 1 || signature % 4) usage();
       break;
     case 'q':	/* quiet */
       verbose = 0;
       break;
     case 'v':	/* version */
     default:
       usage();
       break;
     }
   }

   infile = stdin;
   outfile = stdout;

   if (optind != argc) {
     /* User specified an input file */
     if ((infile = fopen(argv[optind], "rb")) == NULL)
       die("can't open input file %s", argv[optind]);
     optind++;
   }

   if (optind != argc) {
     /* User specified an output file */
     if ((outfile = fopen(argv[optind], "wb")) == NULL)
       die("can't open output file %s", argv[optind]);
     optind++;
   }

   if(optind != argc) usage();

   check_input_and_output_in_binary_mode(infile, outfile);

   if ((infile=seekable(infile))==NULL)
      die("can't seek input");

   scanpages(NULL);

   if (!signature)
      signature = maxpage = pages+(4-pages%4)%4;
   else
      maxpage = pages+(signature-pages%signature)%signature;

   /* rearrange pages */
   writeheader(maxpage, NULL);
   writeprolog();
   writesetup();
   for (currentpg = 0; currentpg < maxpage; currentpg++) {
      int actualpg = currentpg - currentpg%signature;
      switch(currentpg%4) {
      case 0:
      case 3:
	 actualpg += signature-1-(currentpg%signature)/2;
	 break;
      case 1:
      case 2:
	 actualpg += (currentpg%signature)/2;
	 break;
      default: /* Avoid a compiler warning */
         break;
      }
      if (actualpg < pages)
	 writepage(actualpg);
      else
	 writeemptypage();
   }
   writetrailer();

   return 0;
}
