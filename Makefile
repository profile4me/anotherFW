CPP_FLAGS = -g -std=c++11 `root-config --cflags --libs` -lSpectrum -I.  -I${HADDIR}/include -L${HADDIR}/lib -lHydra -lDst -lStart -lRich -lMdc -lTof -lShower -lEmc -lRpc -lWall -lPionTracker -lMdcTrackG -lMdcTrackD -lParticle -lOra
CLASSES = PeriodAssets
OBJECTS = $(patsubst %,obj/%Dict.cxx.o,$(CLASSES)) $(patsubst %,obj/%.cxx.o,$(CLASSES))

all: app

dicts/PeriodAssetsDict.cxx: PeriodAssets.h LinkDef.h
	rootcint -f dicts/PeriodAssetsDict.cxx -c -p PeriodAssets.h LinkDef.h

obj/PeriodAssetsDict.cxx.o: dicts/PeriodAssetsDict.cxx
	g++ $(CPP_FLAGS) -o obj/PeriodAssetsDict.cxx.o -c dicts/PeriodAssetsDict.cxx

obj/PeriodAssets.cxx.o: PeriodAssets.cxx
	g++ $(CPP_FLAGS) -o obj/PeriodAssets.cxx.o -c PeriodAssets.cxx

obj: $(OBJECTS)

# app: dicts/PeriodAssetsDict.cxx main.cc

app: $(OBJECTS) main.cc
	g++ $(CPP_FLAGS) -o app $^