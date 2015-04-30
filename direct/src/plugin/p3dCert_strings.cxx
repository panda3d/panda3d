// Filename: p3dCert_strings.h
// Created by:  rdb (25Mar15)
//
////////////////////////////////////////////////////////////////////
//
// PANDA 3D SOFTWARE
// Copyright (c) Carnegie Mellon University.  All rights reserved.
//
// All use of this software is subject to the terms of the revised BSD
// license.  You should have received a copy of this license along
// with this source code in a file named "LICENSE."
//
////////////////////////////////////////////////////////////////////

#include "p3dCert_strings.h"

// Translations kindly provided by:
// eng: drwr
// nld: rdb
// deu: Sebastian Hoffmann <TheCheapestPixels at googlemail dot com>
// spa: Imanol Celaya <imanol at celaya dot me>
// ita: Flavio Clava
// rus: montreal

const char *language_codes[LI_COUNT] =
  {"en", "nl", "de", "es", "it", "eo", "ru"};

// https://msdn.microsoft.com/en-us/library/windows/desktop/dd318693%28v=vs.85%29.aspx
const unsigned char language_ids[LI_COUNT] =
  {0x09, 0x13, 0x07, 0x0A, 0x10, 0, 0x19};

const char *
run_title[LI_COUNT] = {
  "Run",
  "Uitvoeren",
  "Starten",
  "Ejecutar",
  "Lancia",
  "Lan\304\211i",
  "\320\227\320\260\320\277\321\203\321\201\321\202\320\270\321\202\321\214",
};

const char *
cancel_title[LI_COUNT] = {
  "Cancel",
  "Annuleren",
  "Abbrechen",
  "Cancelar",
  "Cancella",
  "Nuligi",
  "\320\236\321\202\320\274\320\265\320\275\320\260",
};

const char *
show_cert_title[LI_COUNT] = {
  "Show Certificate",
  "Toon Certificaat",
  "Zertifikat anzeigen",
  "Mostrar certificado",
  "Mostra certificato",
  "Montri Ateston",
  "\320\237\320\276\320\272\320\260\320\267\320\260\321\202\321\214 \321\201"
  "\320\265\321\200\321\202\320\270\321\204\320\270\320\272\320\260\321\202",
};

const char *
new_application_title[LI_COUNT] = {
  "New Panda3D Application",
  "Nieuwe Panda3D Applicatie",
  "Neue Panda3D-Anwendung",
  "Nueva aplicaci\303\263n de Panda3D",
  "Nuova applicazione Panda3D",
  "Nova aplika\304\265o de Panda3D",
  "\320\235\320\276\320\262\320\276\320\265 Panda3D-\320\277\321\200\320\270"
  "\320\273\320\276\320\266\320\265\320\275\320\270\320\265",
};

const char *
no_cert_title[LI_COUNT] = {
  "No signature!",
  "Geen handtekening!",
  "Keine Signatur!",
  "Sin firma!",
  "Nessuna firma!",
  "Ne subskribo!",
  "\320\235\320\265\321\202 \320\277\320\276\320\264\320\277\320\270\321\201"
  "\320\270!",
};

const char *
unverified_cert_title[LI_COUNT] = {
  "Unverified signature!",
  "Ongeverifieerde handtekening!",
  "Unbest\303\244tigte Signatur!",
  "Firma sin verificar!",
  "Firma non verificata!",
  "Nekontrolita subskribo!",
  "\320\235\320\265\320\277\321\200\320\276\320\262\320\265\321\200\320\265"
  "\320\275\320\275\320\260\321\217 \320\277\320\276\320\264\320\277\320\270"
  "\321\201\321\214!",
};

const char *
expired_cert_title[LI_COUNT] = {
  "Expired signature!",
  "Verlopen handtekening!",
  "Abgelaufene Signatur!",
  "Firma caducada!",
  "Firma scaduta!",
  "Eksvalidi\304\235inta subskribo!",
  "\320\241\321\200\320\276\320\272 \320\264\320\265\320\271\321\201\321\202"
  "\320\262\320\270\321\217 \320\277\320\276\320\264\320\277\320\270\321\201"
  "\320\270 \320\270\321\201\321\202\321\221\320\272!",
};


const char *
self_signed_cert_text[LI_COUNT] = {
  // eng
  "This Panda3D application uses a self-signed certificate.  "
  "This means the author's name can't be verified, and you have "
  "no way of knowing for sure who wrote it.\n"
  "\n"
  "We recommend you click Cancel to avoid running this application.",

  // nld
  "Deze Panda3D applicatie gebruikt een zelf-getekend certificaat.  "
  "Dit betekent dat de auteursnaam niet kan worden geverifieerd, en het "
  "niet zeker is of de applicatie te vertrouwen is.\n"
  "\n"
  "Het is aanbevolen om op Annuleren te klikken om de applicatie af te "
  "sluiten.",

  // deu
  "Diese Panda3D-Anwendung benutzt ein selbst-signiertes Zertifikat.  Dies "
  "bedeutet, dass weder der Name des Autors \303\274berpr\303\274ft werden "
  "kann, noch dass garantiert werden kann, dass tats\303\244chlich die "
  "angegebene Person diese Anwendung geschrieben hat.\n"
  "\n"
  "Wir empfehlen, dass Sie Abbrechen dr\303\274cken um diese Anwendung nicht "
  "auszuf\303\274hren.",

  // spa
  "Esta aplicaci\303\263n de Panda3D usa un certificado autofirmado.  "
  "Esto significa que el nombre del autor no puede ser verificado y no se "
  "puede conocer seguro quien la ha escrito.\n"
  "\n"
  "Se recomienda cancelar para evitar ejecutar esta aplicaci\303\263n.",

  // ita
  "Questa applicazione Panda3D usa un certificato autofirmato.  Ci\303\262 "
  "significa che il nome dell'autore non pu\303\262 essere verificato, e "
  "che non hai modo di assicurarti circa chi la abbia scritta.\n"
  "\n"
  "Raccomandiamo di cliccare su Cancella per evitare di lanciare questa "
  "applicazione.",

  // epo
  "\304\210i tiu aplika\304\265o de Panda3D uzas memsubskribitan ateston.  "
  "Tio signifas  ke la nomo de la verkanto ne povas esti kontrolita, kaj vi "
  "ne havas certan scimanieron pri la vera verkanto de la aplika\304\265o.\n"
  "\n"
  "Ni rekomendas ke vi premas la butonon 'Nuligi' por eviti lan\304\211on de "
  "\304\211i tiu aplika\304\265o.",

  // rus
  "\320\255\321\202\320\276 \320\277\321\200\320\270\320\273\320\276\320\266"
  "\320\265\320\275\320\270\320\265 \320\270\321\201\320\277\320\276\320\273"
  "\321\214\320\267\321\203\320\265\321\202 \321\201\320\260\320\274\320\276"
  "\320\267\320\260\320\262\320\265\321\200\320\265\320\275\320\275\321\213"
  "\320\271 \321\201\320\265\321\200\321\202\320\270\321\204\320\270\320\272"
  "\320\260\321\202.  \320\255\321\202\320\276 \320\276\320\267\320\275"
  "\320\260\321\207\320\260\320\265\321\202, \321\207\321\202\320\276 "
  "\320\270\320\274\321\217 \320\260\320\262\321\202\320\276\321\200\320\260 "
  "\320\275\320\265 \320\274\320\276\320\266\320\265\321\202 \320\261\321\213"
  "\321\202\321\214 \320\277\320\276\320\264\321\202\320\262\320\265\320\266"
  "\320\264\320\265\320\275\320\276, \320\270 \320\262\321\213 \320\275"
  "\320\265 \320\274\320\276\320\266\320\265\321\202\320\265 \320\267\320\275"
  "\320\260\321\202\321\214, \320\272\321\202\320\276 \320\275\320\260"
  "\320\277\320\270\321\201\320\260\320\273 \321\215\321\202\320\276 \320\277"
  "\321\200\320\270\320\273\320\276\320\266\320\265\320\275\320\270\320\265.\n"
  "\n"
  "\320\234\321\213 \321\200\320\265\320\272\320\276\320\274\320\265\320\275"
  "\320\264\321\203\320\265\320\274 \320\262\320\260\320\274 \320\275\320\260"
  "\320\266\320\260\321\202\321\214 \"\320\236\321\202\320\274\320\265"
  "\320\275\320\260\" \320\270 \320\275\320\265 \320\267\320\260\320\277"
  "\321\203\321\201\320\272\320\260\321\202\321\214 \321\215\321\202\320\276 "
  "\320\277\321\200\320\270\320\273\320\276\320\266\320\265\320\275\320\270"
  "\320\265.",
};


const char *
unknown_auth_cert_text[LI_COUNT] = {
  "This Panda3D application has been signed, but we don't recognize "
  "the authority that verifies the signature.  This means the author's "
  "name can't be verified, and you have no way of knowing "
  "for sure who wrote it.\n"
  "\n"
  "We recommend you click Cancel to avoid running this application.",

   // nld
  "Deze Panda3D applicatie is ondertekend, maar de certificaatautoriteit "
  "die het certificaat heeft uitgegeven wordt niet herkend.  Dit betekent "
  "dat de auteursnaam niet te vertrouwen is, en het niet zeker is wie de "
  "applicatie gemaakt heeft.\n"
  "\n"
  "Het is aanbevolen om op Annuleren te klikken om de applicatie af te "
  "sluiten.",

  // deu
  "Diese Panda3D-Anwendung wurde signiert, aber die "
  "\303\234berpr\303\274fungsstelle wurde nicht anerkannt.  Dies bedeutet, "
  "dass weder der Name des Autors \303\274berpr\303\274ft werden kann, noch "
  "dass garantiert werden kann, dass tats\303\244chlich die angegebene "
  "Person diese Anwendung geschrieben hat.\n"
  "\n"
  "Wir empfehlen, dass Sie Abbrechen dr\303\274cken um diese Anwendung nicht "
  "auszuf\303\274hren.",

  // spa
  "Esta aplicaci\303\263n de Panda3D esta firmada, pero no reconocemos la "
  "autoridad que la verifica.  Esto significa que el nombre del autor no "
  "puede ser verificado y no se puede conocer seguro quien la ha escrito.\n"
  "\n"
  "Se recomienda cancelar para evitar ejecutar esta aplicaci\303\263n.",

  // ita
  "Questa applicazione Panda3D \303\250 stata firmata, ma non riconosciamo "
  "l'autorit\303\240 che verifica la firma. Ci\303\262 significa che il "
  "nome dell'autore non pu\303\262 essere verificato, e che non hai modo "
  "di assicurarti circa chi la abbia scritta.\n"
  "\n"
  "Raccomandiamo di cliccare su Cancella per evitare di lanciare questa "
  "applicazione.",

  // epo
  "\304\210i tiu aplika\304\265o estas subskribita, sed ni ne rekonas "
  "la a\305\255toritaton, kiu kontrolas la subskribon. Tio signifas ke la "
  "nomo de la verkanto ne povas esti konfidata, kaj vi ne havas certan "
  "scimanieron pri la vera verkanto de la aplika\304\265o.\n"
  "\n"
  "Ni rekomendas ke vi premas la butonon 'Nuligi' por eviti lan\304\211on "
  "de \304\211i tiu aplika\304\265o.",

  // rus
  "\320\243 \321\215\321\202\320\276\320\263\320\276 \320\277\321\200\320\270"
  "\320\273\320\276\320\266\320\265\320\275\320\270\321\217 \320\265\321\201"
  "\321\202\321\214 \320\277\320\276\320\264\320\277\320\270\321\201\321\214,"
  " \320\277\320\276 \320\272\320\276\321\202\320\276\321\200\320\276\320\271"
  " \320\274\321\213 \320\275\320\265 \320\274\320\276\320\266\320\265"
  "\320\274 \321\200\320\260\321\201\320\277\320\276\320\267\320\275\320\260"
  "\321\202\321\214 \320\262\320\273\320\260\320\264\320\265\320\273\321\214"
  "\321\206\320\260.  \320\255\321\202\320\276 \320\276\320\267\320\275"
  "\320\260\321\207\320\260\320\265\321\202, \321\207\321\202\320\276 "
  "\320\270\320\274\321\217 \320\260\320\262\321\202\320\276\321\200\320\260 "
  "\320\275\320\265 \320\274\320\276\320\266\320\265\321\202 \320\261\321\213"
  "\321\202\321\214 \320\276\320\277\321\200\320\265\320\264\320\265\320\273"
  "\320\265\320\275\320\276, \320\270 \320\275\320\265\320\262\320\276"
  "\320\267\320\274\320\276\320\266\320\275\320\276 \321\202\320\276\321\207"
  "\320\275\320\276 \321\203\320\267\320\275\320\260\321\202\321\214, "
  "\320\272\321\202\320\276 \320\275\320\260\320\277\320\270\321\201\320\260"
  "\320\273 \321\215\321\202\320\276 \320\277\321\200\320\270\320\273\320\276"
  "\320\266\320\265\320\275\320\270\320\265.\n"
  "\n"
  "\320\234\321\213 \321\200\320\265\320\272\320\276\320\274\320\265\320\275"
  "\320\264\321\203\320\265\320\274 \320\262\320\260\320\274 \320\275\320\260"
  "\320\266\320\260\321\202\321\214 \"\320\236\321\202\320\274\320\265"
  "\320\275\320\260\" \320\270 \320\275\320\265 \320\267\320\260\320\277"
  "\321\203\321\201\320\272\320\260\321\202\321\214 \321\215\321\202\320\276 "
  "\320\277\321\200\320\270\320\273\320\276\320\266\320\265\320\275\320\270"
  "\320\265.",
};


const char *
verified_cert_text[LI_COUNT] = {
  // eng
  "This Panda3D application has been signed by %s.  "
  "If you trust %s, then click the Run button below "
  "to run this application on your computer.  This will also "
  "automatically approve this and any other applications signed by "
  "%s in the future.\n"
  "\n"
  "If you are unsure about this application, "
  "you should click Cancel instead.",

  // nld
  "Deze Panda3D applicatie is ondertekend door %s.  "
  "Als u %s vertrouwt, klik dan de onderstaande knop Uitvoeren om de applicatie "
  "op uw computer uit te voeren.  Dit zal ook deze en andere door %s getekende "
  "applicaties in het vervolg toestaan.\n"
  "\n"
  "Als u niet zeker bent over deze applicatie is het aanbevolen om op "
  "Annuleren te klikken.",

  // deu
  "Diese Panda3D-Anwendung wurde von %s signiert.  Falls Sie %s vertrauen, "
  "dr\303\274cken Sie den Starten-Knopf, um diese Anwendung auszuf\303\274hren.  "
  "Zudem werden in der Zukunft diese und alle anderen von %s signierten "
  "Anwendungen automatisch als vertrauensw\303\274rdig anerkannt.\n"
  "\n"
  "Falls Sie sich unsicher \303\274ber diese Anwendung sind, sollten Sie "
  "Abbrechen dr\303\274cken.",

  // spa
  "Esta aplicaci\303\263n de Panda3D ha sido firmada por %s.  Si se considera %s"
  "de confianza el bot\303\263n inferior ejecutar\303\241 la aplicaci\303\263n."
  "Esto validara esta y cualquier otra applizacion firmada por %s en el futuro.\n"
  "\n"
  "Si se duda de la aplicaci\303\263nse recomienda cancelar."

  // ita
  "Questa applicazione Panda3D \303\250 stata firmata da %s.  Se %s \303\250 "
  "un'entit\303\240 fidata, allora clicca il bottone Lancia sottostante per "
  "lanciare questa applicazione sul tuo computer. Inoltre, ci\303\262 "
  "approver\303\240 automaticamente questa e ogni altra applicazione "
  "firmata da %s in futuro.\n"
  "\n"
  "Se non sei sicuro circa questa applicazione, dovresti invece cliccare "
  "su Cancella.",

  // epo
  "\304\210i tiu aplika\304\265o estas subskribita de %s.  "
  "Se %s estas fidinda la\305\255 vi, premu la butonon 'Lan\304\211i' sube por "
  "lan\304\211i \304\211i tiun aplika\304\265on per via komputilo. Anka\305\255 tio a\305\255tomate estonece "
  "aprobos \304\211i tiun kaj alian ajn aplika\304\265on, kiu estas subskribita de %s.\n"
  "\n"
  "Se vi ne estas certa pri \304\211i tiu aplika\304\265o, "
  "vi anstata\305\255e povus premi la butonon 'Nuligi'.",

  // rus
  "\320\255\321\202\320\276 \320\277\321\200\320\270\320\273\320\276\320\266"
  "\320\265\320\275\320\270\320\265 \320\277\320\276\320\264\320\277\320\270"
  "\321\201\320\260\320\275\320\276 %s.  "
  "\320\225\321\201\320\273\320\270 \320\262\321\213 \320\264\320\276\320\262"
  "\320\265\321\200\321\217\320\265\321\202\320\265 %s, \320\275\320\260"
  "\320\266\320\274\320\270\321\202\320\265 \320\272\320\275\320\276\320\277"
  "\320\272\321\203 \"\320\227\320\260\320\277\321\203\321\201\321\202"
  "\320\270\321\202\321\214\" \320\262\320\275\320\270\320\267\321\203, "
  "\321\207\321\202\320\276\320\261\321\213 \320\267\320\260\320\277\321\203"
  "\321\201\321\202\320\270\321\202\321\214 \321\215\321\202\320\276 \320\277"
  "\321\200\320\270\320\273\320\276\320\266\320\265\320\275\320\270\320\265 "
  "\320\275\320\260 \320\262\320\260\321\210\320\265\320\274 \320\272\320\276"
  "\320\274\320\277\321\214\321\216\321\202\320\265\321\200\320\265. \320\255"
  "\321\202\320\276 \321\202\320\260\320\272\320\266\320\265 \320\260\320\262"
  "\321\202\320\276\320\274\320\260\321\202\320\270\321\207\320\265\321\201"
  "\320\272\320\270 \320\277\320\276\320\264\321\202\320\262\320\265\321\200"
  "\320\264\320\270\321\202 \321\215\321\202\320\276 \320\270 \320\264"
  "\321\200\321\203\320\263\320\270\320\265 \320\277\321\200\320\270\320\273"
  "\320\276\320\266\320\265\320\275\320\270\321\217, \320\275\320\260\320\277"
  "\320\270\321\201\320\260\320\275\320\275\321\213\320\265 %s \320\262 "
  "\320\261\321\203\320\264\321\203\321\211\320\265\320\274.\n"
  "\n"
  "\320\225\321\201\320\273\320\270 \320\262\321\213 \320\275\320\265 "
  "\321\203\320\262\320\265\321\200\320\265\320\275\321\213 \320\262 \321\215"
  "\321\202\320\276\320\274 \320\277\321\200\320\270\320\273\320\276\320\266"
  "\320\265\320\275\320\270\320\270, \320\275\320\260\320\266\320\274\320\270"
  "\321\202\320\265 \320\272\320\275\320\276\320\277\320\272\321\203 \""
  "\320\236\321\202\320\274\320\265\320\275\320\260\".",
};


const char *
expired_cert_text[LI_COUNT] = {
  // eng
  "This Panda3D application has been signed by %s, "
  "but the certificate has expired.\n"
  "\n"
  "You should check the current date set on your computer's clock "
  "to make sure it is correct.\n"
  "\n"
  "If your computer's date is correct, we recommend "
  "you click Cancel to avoid running this application.",

  // nld
  "Deze Panda3D applicatie is ondertekend door %s, maar de geldigheidsdatum "
  "van het certificaat is verstreken.\n"
  "\n"
  "Controleer de datum op uw computerklok om te zorgen dat deze juist is "
  "ingesteld.\n"
  "\n"
  "Als de datum op uw computer juist is, is het aanbevolen om op Annuleren te "
  "klikken om de applicatie af te sluiten.",

  // deu
  "Diese Anwendung wurde von %s signiert, aber das Zertifikat ist abgelaufen.\n"
  "\n"
  "Sie sollten die aktuelle Uhrzeit auf Ihrem Computer "
  "\303\274berpr\303\274fen, um sicherzugehen, dass sie korrekt ist.\n"
  "\n"
  "Falls die Uhrzeit auf Ihrem Computer korrekt ist, empfehlen wir Ihnen "
  "Abbrechen zu dr\303\274cken.",

  // spa
  "Esta aplicaci\303\263n Panda3D ha sido firmada por %s pero el certificado ha"
  "caducado.\n"
  "\n"
  "Se recomienda comprobar la fecha del reloj.\n"
  "\n"
  "Si la fecha del reloj es correcta se recomienda cancelar.",

  // ita
  "Questa applicazione Panda3D \303\250 stata firmata da %s, ma il "
  "certificato \303\250 scaduto.\n"
  "\n"
  "Dovresti controllare la data attuale impostata nell'orologio del tuo "
  "computer per assicurarti che sia corretta.\n"
  "\n"
  "Se la data del tuo computer \303\250 corretta, raccomandiamo di cliccare "
  "Cancella per evitare di lanciare questa applicazione."

  // epo
  "\304\210i tiu aplika\304\265o de Panda3D estas subskribita de %s, "
  "sed la atesto eksvalidi\304\235is.\n"
  "\n"
  "Vi povus kontroli la aktualan daton, kiu estas agordata sur la horlo\304\235o de "
  "via komputilo por certigi ke \304\235i estas korekta.\n"
  "\n"
  "Se la dato de via komputilo estas korekta, ni rekomendas ke vi premas la "
  "butonon 'Nuligi' por eviti lan\304\211on de \304\211i tiu aplika\304\265o.",

  // rus
  "\320\255\321\202\320\276 \320\277\321\200\320\270\320\273\320\276\320\266"
  "\320\265\320\275\320\270\320\265 \320\277\320\276\320\264\320\277\320\270"
  "\321\201\320\260\320\275\320\276 %s, \320\276\320\264\320\275\320\260"
  "\320\272\320\276 \321\201\321\200\320\276\320\272 \320\264\320\265\320\271"
  "\321\201\321\202\320\262\320\270\321\217 \321\201\320\265\321\200\321\202"
  "\320\270\321\204\320\270\320\272\320\260\321\202\320\260 \320\270\321\201"
  "\321\202\321\221\320\272.\n"
  "\n"
  "\320\237\321\200\320\276\320\262\320\265\321\200\321\214\321\202\320\265, "
  "\320\277\320\276\320\266\320\260\320\273\321\203\320\271\321\201\321\202"
  "\320\260, \320\264\320\260\321\202\321\203 \320\275\320\260 \320\262"
  "\320\260\321\210\320\265\320\274 \320\272\320\276\320\274\320\277\321\214"
  "\321\216\321\202\320\265\321\200\320\265.\n"
  "\n"
  "\320\225\321\201\320\273\320\270 \320\275\320\260 \320\262\320\260\321\210"
  "\320\265\320\274 \320\272\320\276\320\274\320\277\321\214\321\216\321\202"
  "\320\265\321\200\320\265 \321\203\321\201\321\202\320\260\320\275\320\276"
  "\320\262\320\273\320\265\320\275\320\276 \320\277\321\200\320\260\320\262"
  "\320\270\320\273\321\214\320\275\320\276\320\265 \320\262\321\200\320\265"
  "\320\274\321\217, \320\274\321\213 \321\200\320\265\320\272\320\276"
  "\320\274\320\265\320\275\320\264\321\203\320\265\320\274 \320\262\320\260"
  "\320\274 \320\275\320\260\320\266\320\260\321\202\321\214 \320\272\320\275"
  "\320\276\320\277\320\272\321\203 \"\320\236\321\202\320\274\320\265"
  "\320\275\320\260\" \320\270 \320\275\320\265 \320\267\320\260\320\277"
  "\321\203\321\201\320\272\320\260\321\202\321\214 \321\215\321\202\320\276 "
  "\320\277\321\200\320\270\320\273\320\276\320\266\320\265\320\275\320\270"
  "\320\265.",
};


const char *
generic_error_cert_text[LI_COUNT] = {
  // eng
  "This Panda3D application has been signed, but there is a problem "
  "with the certificate (OpenSSL error code %d).\n"
  "\n"
  "We recommend you click Cancel to avoid running this application.",

  // nld
  "Deze Panda3D applicatie is ondertekend, maar er is een probleem "
  "met het certificaat opgetreden (OpenSSL foutcode %d).\n"
  "\n"
  "Het is aanbevolen om op Annuleren te klikken om de applicatie af te "
  "sluiten.",

  // deu
  "Diese Panda3D-Anwendung wurde signiert, aber es gibt ein Problem mit "
  "dem Zertifikat (OpenSSL Fehlercode %d).\n"
  "\n"
  "Wir empfehlen, dass Sie Abbrechen dr\303\274cken um diese Anwendung nicht "
  "auszuf\303\274hren.",

  // spa
  "Esta aplicaci\303\263n de Panda3D esta firmada pero hay un problema con el "
  "certificado (Error de OpenSSL %d).\n"
  "\n"
  "Se recomienda cancelar para evitar ejecutar esta aplicaci\303\263n.",

  // ita
  "Questa applicazione Panda3D \303\250 stata firmata, ma c'\303\250 un "
  "problema col certificato (codice di errore OpenSSL %s).\n"
  "\n"
  "Raccomandiamo di cliccare su Cancella per evitare di lanciare questa "
  "applicazione.",

  // epo
  "\304\210i tiu aplika\304\265o de Panda3D estas subskribita, sed la "
  "atesto havas problemon (OpenSSL erarkodo %d).\n"
  "\n"
  "Ni rekomendas ke vi premas la butonon 'Nuligi' por eviti lan\304\211on "
  "de \304\211i tiu aplika\304\265o.",

  // rus
  "\320\243 \321\215\321\202\320\276\320\263\320\276 \320\277\321\200\320\270"
  "\320\273\320\276\320\266\320\265\320\275\320\270\321\217 \320\265\321\201"
  "\321\202\321\214 \320\277\320\276\320\264\320\277\320\270\321\201\321\214,"
  " \320\275\320\276 \320\262\320\276\320\267\320\275\320\270\320\272\320\273"
  "\320\260 \320\277\321\200\320\276\320\261\320\273\320\265\320\274\320\260 "
  "\321\201 \321\201\320\265\321\200\321\202\320\270\321\204\320\270\320\272"
  "\320\260\321\202\320\276\320\274 (\320\232\320\276\320\264 \320\276"
  "\321\210\320\270\320\261\320\272\320\270 OpenSSL %d).\n"
  "\n"
  "\320\234\321\213 \321\200\320\265\320\272\320\276\320\274\320\265\320\275"
  "\320\264\321\203\320\265\320\274 \320\262\320\260\320\274 \320\275\320\260"
  "\320\266\320\260\321\202\321\214 \"\320\236\321\202\320\274\320\265"
  "\320\275\320\260\" \320\270 \320\275\320\265 \320\267\320\260\320\277"
  "\321\203\321\201\320\272\320\260\321\202\321\214 \321\215\321\202\320\276 "
  "\320\277\321\200\320\270\320\273\320\276\320\266\320\265\320\275\320\270"
  "\320\265.",
};


const char *
no_cert_text[LI_COUNT] = {
  "This Panda3D application has not been signed.  This means you have "
  "no way of knowing for sure who wrote it.\n"
  "\n"
  "Click Cancel to avoid running this application.",

  // nld
  "Deze Panda3D applicatie is niet ondertekend.  Dit betekent dat het niet "
  "mogelijk is om de auteur te verifi\303\253ren.\n"
  "\n"
  "Klik op Annuleren om de applicatie af te sluiten.",

  // deu
  "Diese Panda3D-Anwendung wurde nicht signiert.  Es gibt keine "
  "M\303\266glichkeit festzustellen, wer diese entwickelt hat.\n"
  "\n"
  "Dr\303\274cken Sie Abbrechen, um diese Anwendung nicht auszuf\303\274hren.",

  // spa
  "Esta aplicaci\303\263n de Panda3D no esta firmada, no hay forma de conocer "
  "quien la ha escrito.\n"
  "\n"
  "Cancelar para evitar ejecutar la aplicaci\303\263n.",

  // ita
  "Questa applicazione Panda3D non \303\250 stata firmata.  Ci\303\262 "
  "significa che non hai modo di assicurarti circa chi la abbia scritta.\n"
  "\n"
  "Clicca Cancella per evitare di lanciare questa applicazione.",

  // epo
  "\304\210i tiu aplika\304\265o de Panda3D ne estas subskribita.  Tio "
  "signifas ke vi ne  havas certan scimanieron pri la vera verkanto de "
  "la aplika\304\265o.\n"
  "\n"
  "Premu la butonon 'Nuligi' por eviti lan\304\211on de \304\211i tiu "
  "aplika\304\265o.",

  // rus
  "\320\243 \321\215\321\202\320\276\320\263\320\276 \320\277\321\200\320\270"
  "\320\273\320\276\320\266\320\265\320\275\320\270\321\217 \320\275\320\265"
  "\321\202 \320\277\320\276\320\264\320\277\320\270\321\201\320\270. "
  "\320\255\321\202\320\276 \320\276\320\267\320\275\320\260\321\207\320\260"
  "\320\265\321\202, \321\207\321\202\320\276 \320\262\321\213 \320\275"
  "\320\265 \320\274\320\276\320\266\320\265\321\202\320\265 \320\267\320\275"
  "\320\260\321\202\321\214, \320\272\321\202\320\276 \320\265\320\263"
  "\320\276 \320\275\320\260\320\277\320\270\321\201\320\260\320\273.\n"
  "\n"
  "\320\235\320\260\320\266\320\274\320\270\321\202\320\265 \"\320\236"
  "\321\202\320\274\320\265\320\275\320\260\", \321\207\321\202\320\276"
  "\320\261\321\213 \320\275\320\265 \320\267\320\260\320\277\321\203\321\201"
  "\320\272\320\260\321\202\321\214 \320\277\321\200\320\270\320\273\320\276"
  "\320\266\320\265\320\275\320\270\320\265.",
};
