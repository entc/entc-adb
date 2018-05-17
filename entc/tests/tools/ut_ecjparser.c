
#include "tools/ecjson.h"
#include "tests/ecenv.h"

//=============================================================================

static void __STDCALL onLine (void* ptr, const char* line)
{

}

//---------------------------------------------------------------------------

static int __STDCALL test_stdjparser_test1 (void* ptr, TestEnvContext tctx, EcErr err)
{
  const EcString text = "{\"text\":\"Mobilfunk\\nMindestlaufzeit &\\nK\\u00fcndigungsfrist\\n& automatische\\nVertragsverl\\u00e4ngerung\\nMonatlicher\\nBasispreis brutto Monatlicher\\nBasispreis netto\\n24 Monate \\/\\n3 Monate \\/\\n12 Monate 36,99 \\u20ac 31,0840 \\u20ac\\nVodafone Secure Net, sicher sorglos surfen\\nDer netzbasierte Dienst \\\"Vodafone Secure Net\\\" ist Teil Ihres\\nMobilfunkvertrags. Beim Surfen Innerhalb des Mobilfunknetzes\\nder Vodafone GmbH sind Sie damit vor den erkennbaren\\nGefahren des Internet gesch\\u00fctzt. Sie nutzen den Dienst 3 Monate\\nkostenfrei, danach kostet die Nutzung 0,99 Euro\\/Monat. Sie\\nk\\u00f6nnen den Dienst jederzeit direkt \\u00fcber das Vodafone-Secure-Net\\nPortal k\\u00fcndigen. -\\/\\n0 Monate 0,99 \\u20ac 0,8319 \\u20ac\",\"#img_0\":\"- I - v w I I I '- - - - \\u2018 Mindestlaufzeit & Monatlicher Monatlicher\\nKilndigungsfrist Basispreis brutto Basispreis netto\\n& automatische\\nVertragsverl\\u00e9ingerung\\nNeuauftrag mit Portierung f\\ufb02r Teilnehmer 0176 \\/ 66365813\\nVodafone Smart L mit Basic Phone 1) 2) 3) 4) 5) 6) 24 Monate \\/ 36199 \\u20ac 3110840 \\u20ac\\nProdukt im Angebot seit 01.08.2017 3 Monate \\/\\n- Sie surfen mit bis zu 500 Mbitls im Download und 50 Mbitl 12 Monate\\n5 im Upload.\\n- Die Sprachminuten, SMS und das Datenvolumen sind ohne\\nMehrkosten im EU-Ausland nutzbar.\\n- Zum schnelleren Surfen schalten wir nach Verbrauch des\\nHighspeed-Volumens bis zu 3 Mal je 100 MB fUr 2 Euro\\nfrei. Sie werden vorher informiert und k\\u00e9nnen per SMS\\nablehnen. Dann surfen Sie mit bis zu 32 kbit\\/s Mehr Infos auf:\\nwww vndafnne de\\/sneednn\\n\\n\",\"Tarif\":\"Smart L mit Basic Phone\",\"Grundgeb\\u00fchr\":\"36,99\"}";
  
  EcUdc node = ecjson_read_s (text, NULL);
  
  return 0;
}

//---------------------------------------------------------------------------

int main(int argc, char* argv[])
{
  TestEnv te = testenv_create ();
  
  testenv_reg (te, "LineParser Test1", NULL, NULL, test_stdjparser_test1);
  
  testenv_run (te);
  
  return testenv_destroy (&te);
}

//---------------------------------------------------------------------------
