ANAMAIN_SOURCES       =	\
			Analysis_Object.C \
			Primitive_Analysis.C \
			Analysis_Handler.C

ANAMAIN_ADDS          =	\
			Analysis_Object.H \
			Primitive_Analysis.H \
			Analysis_Handler.H

if USING__Analysis
pkglib_LTLIBRARIES    =	libSherpaAnalysis.la

libSherpaAnalysis_la_SOURCES  =	$(ANAMAIN_SOURCES)

localinc_HEADERS      = $(ANAMAIN_ADDS)

libSherpaAnalysis_la_LIBADD   =	-L../Tools \
	-L../Triggers -L../Observables -L../Analyses \
	-lSherpaAnalysisTools -lSherpaAnalysisTrigger \
	-lSherpaObservables -lSherpaAnalyses

libSherpaAnalysis_la_CXXFLAGS = $(AM_CXXFLAGS)

libSherpaAnalysis_la_CPPFLAGS = $(AM_CPPFLAGS)

ANAMAIN_EXTRA_DIST = $(ANAMAIN_ADDS)
else
ANAMAIN_EXTRA_DIST = $(ANAMAIN_SOURCES) $(ANAMAIN_ADDS)
endif

EXTRA_DIST = $(ANAMAIN_EXTRA_DIST)
