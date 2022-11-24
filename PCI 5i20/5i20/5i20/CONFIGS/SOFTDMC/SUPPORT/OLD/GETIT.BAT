COPY \tp7\tpmesa\include\smotbits.pas
COPY \tp7\tpmesa\include\smotparm.pas
COPY \tp7\tpmesa\include\swmotlow.pas
COPY \softdmc\include.inc
COPY \tp7\tpmesa\4i34\newdmc.pas
COPY \tp7\tpmesa\4i34\newdmc.exe
call zip -a dmcsupp.zip *.pas
call zip -a dmcsupp.zip *.exe
call zip -a dmcsupp.zip *.inc
del *.pas
del *.exe
del *.inc
