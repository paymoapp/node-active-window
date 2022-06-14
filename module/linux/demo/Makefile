CXX=g++
RM=rmdir -R

LIBDIR=../src
BUILDDIR=build

SRCS=$(LIBDIR)/ActiveWindow.cpp
OBJS=$(patsubst $(LIBDIR)/%.cpp,$(BUILDDIR)/%.o,$(SRCS))
LDOPTS=-lX11

all: demo

demo: demo.obj lib $(BUILDDIR)/
	@echo Building demo.exe
	$(CXX) -o $(BUILDDIR)/$@ $(OBJS) $(BUILDDIR)/demo.o $(LDOPTS)

demo.obj: main.cpp ../src/ActiveWindow.h $(BUILDDIR)/
	@echo Building demo
	$(CXX) -c -o $(BUILDDIR)/demo.o main.cpp

lib: $(OBJS)
	@echo Built library

$(BUILDDIR)/%.o: $(LIBDIR)/%.cpp $(LIBDIR)/%.h $(BUILDDIR)/
	@echo Building $<
	$(CXX) -c -o $@ $<

$(BUILDDIR)/:
	@echo Creating build directory
	@mkdir $(BUILDDIR)

clean:
	$(RM) $(BUILDDIR)

run:
	$(BUILDDIR)/demo