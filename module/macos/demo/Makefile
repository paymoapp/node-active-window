CXX=clang
RM=rm -R

LIBDIR=../src
BUILDDIR=build

CXXFLAGS=-ObjC++ -std=c++20
LDOPTS=-lc++ -framework Foundation -framework AppKit -framework ApplicationServices

all: demo

demo: demo.obj lib $(BUILDDIR)/
	@echo Building demo executable
	$(CXX) -o $(BUILDDIR)/$@ $(BUILDDIR)/ActiveWindow.o $(BUILDDIR)/base64.o $(BUILDDIR)/demo.o $(LDOPTS)

demo.obj: main.mm $(LIBDIR)/ActiveWindow.h $(BUILDDIR)
	$(CXX) $(CXXFLAGS) -c -o $(BUILDDIR)/demo.o main.mm

lib: $(BUILDDIR)/ActiveWindow.o $(BUILDDIR)/base64.o
	@echo Built library

$(BUILDDIR)/ActiveWindow.o: $(LIBDIR)/ActiveWindow.mm $(LIBDIR)/ActiveWindow.h $(BUILDDIR)/
	@echo Building $<
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILDDIR)/base64.o: $(LIBDIR)/base64.cpp $(LIBDIR)/base64.h $(BUILDDIR)/
	@echo Building $<
	$(CXX) $(CXXFLAGS) -c -o $@ $<

$(BUILDDIR)/:
	@echo Creating build directory
	@mkdir $(BUILDDIR)

clean:
	$(RM) $(BUILDDIR)

run:
	$(BUILDDIR)/demo $(MODE)
