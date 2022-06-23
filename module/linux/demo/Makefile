CXX=g++
RM=rmdir -R

LIBDIR=../src
BUILDDIR=build

SRCS=$(LIBDIR)/ActiveWindow.cpp $(LIBDIR)/base64.cpp $(LIBDIR)/ConvertUTF.c
OBJS=$(patsubst $(LIBDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))
LDOPTS=-lX11 -licuuc -lpthread
CXXOPTS=--std=c++17

all: demo

demo: demo.obj lib $(BUILDDIR)/
	@echo Building demo executable
	$(CXX) -o $(BUILDDIR)/$@ $(OBJS) $(BUILDDIR)/demo.o $(LDOPTS)

demo.obj: main.cpp $(LIBDIR)/ActiveWindow.h $(BUILDDIR)/
	@echo Building demo
	$(CXX) $(CXXOPTS) -c -o $(BUILDDIR)/demo.o main.cpp

lib: $(OBJS)
	@echo Built library

$(BUILDDIR)/%.o: $(LIBDIR)/%.cpp $(LIBDIR)/%.h $(BUILDDIR)/
	@echo Building $<
	$(CXX) $(CXXOPTS) -c -o $@ $<

$(BUILDDIR)/:
	@echo Creating build directory
	@mkdir $(BUILDDIR)

clean:
	$(RM) $(BUILDDIR)

run:
	$(BUILDDIR)/demo $(MODE)
