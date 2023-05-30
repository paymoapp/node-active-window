CXX=g++
RM=rm -R

LIBDIR=../src
BUILDDIR=build

CXXFLAGS=--std=c++17
LDOPTS=-lX11 -lpthread

SRCS=$(LIBDIR)/ActiveWindow.cpp $(LIBDIR)/IconCache.cpp $(LIBDIR)/SimpleIni/ConvertUTF.c $(LIBDIR)/base64/base64.cpp $(LIBDIR)/lodepng/lodepng.cpp
OBJS=$(addsuffix .o,$(addprefix $(BUILDDIR)/,$(notdir $(basename $(SRCS)))))

define build-cpp
$(BUILDDIR)/$(basename $(notdir $(1))).o: $(1) $(addsuffix .h,$(basename $(1))) $(BUILDDIR)/
	@echo Bulding $(1)
	$(CXX) $(CXXFLAGS) -c -o $(BUILDDIR)/$(basename $(notdir $(1))).o $(1)
endef

all: $(BUILDDIR)/demo

$(BUILDDIR)/demo: $(BUILDDIR)/demo.o lib
	@echo Linking demo executable
	$(CXX) -o $@ $(OBJS) $(BUILDDIR)/demo.o $(LDOPTS)

lib: $(OBJS)
	@echo Built library

$(BUILDDIR)/demo.o: main.cpp $(LIBDIR)/ActiveWindow.h $(BUILDDIR)/
	@echo Building main.cpp for demo
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(foreach _t,$(SRCS),$(eval $(call build-cpp,$(_t))))

$(BUILDDIR)/:
	@echo Creating build directory
	@mkdir $(BUILDDIR)

clean:
	$(RM) $(BUILDDIR)

run:
	$(BUILDDIR)/demo $(MODE)
