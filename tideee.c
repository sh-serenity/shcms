#include <tidy.h>
#include <tidybuffio.h>
#include <stdio.h>
#include <errno.h>
void main()
{
  TidyBuffer output = {0};
  TidyBuffer errbuf = {0};
  char *input = (char *)malloc(200);
  int rc = -1;
  Bool ok;
  TidyDoc tdoc = tidyCreate(); // Initialize "document
  input = "<br>Начнем с телятины, Ее необходимо обмыть холодной водой, затем наоезать кубиками&nbsp; Затем оставляем телятину в сторону, и быстренько нарезаем лук. Затем жарим е<p>";
  ok = tidyOptSetBool(tdoc, TidyXhtmlOut, yes); // Convert to XHTML
  if (ok)
    rc = tidySetErrorBuffer(tdoc, &errbuf); // Capture diagnostics
  if (rc >= 0)
    rc = tidyParseString(tdoc, input); // Parse the input
//  if (rc >= 0)
//    rc = tidyOptSetBool(tdoc, TidyMark, no);
//  if (rc >= 0)
//    rc = tidyOptSetInt(tdoc, TidyWrapLen, 0);
  if (rc >= 0)
    rc = tidyOptSetInt(tdoc, TidyBodyOnly, 1);
//  if (rc >= 0)
//    tidySetCharEncoding(tdoc, "utf8");
//  if (rc >= 0)
//    rc = tidyOptSetInt(tdoc, TidyDoctypeMode, TidyDoctypeStrict);
  if (rc >= 0)
    rc = tidyCleanAndRepair(tdoc); // Tidy it up!
  if (rc >= 0)
    rc = tidyRunDiagnostics(tdoc); // Kvetch
//  if (rc > 1)                      // If error, force output.
//    rc = (tidyOptSetBool(tdoc, TidyForceOutput, yes) ? rc : -1);
//  if (ok)
//    rc = tidySetErrorBuffer(tdoc, &errbuf); // Capture diagnostics
q
if ( rc >= 0 )
  {
    if ( rc > 0 )
      printf( "\nDiagnostics:\n\n%s", errbuf.bp );
    printf( "\nAnd here is the result:\n\n%s", output.bp );
  }
  else
    printf( "A severe error (%d) occurred.\n", rc );

  tidyBufFree( &output );
  tidyBufFree( &errbuf );
  tidyRelease( tdoc );
}