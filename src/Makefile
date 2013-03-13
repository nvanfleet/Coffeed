PROGS     = coffeed

INSTDIR   = $(prefix)/usr/sbin
INSTMODE  = 0755
INSTOWNER = root
INSTGROUP = root

OBJS = main.o Logger.o Sensor.o TypeK.o PID.o Heater-pwm.o Server.o

all: $(PROGS)
$(PROGS): $(OBJS)
	$(CXX) $(CFLAGS) -lpthread $(LDFLAGS) $^ $(LDLIBS) -o $@
	$(STRIP) $@

%.o: %.c
	$(CXX) -c $(CFLAGS) $^ -o $@

install: $(PROGS)
	$(INSTALL) -d $(INSTDIR)
	$(INSTALL) -m $(INSTMODE) -o $(INSTOWNER) -g $(INSTGROUP) $(PROGS) $(INSTDIR)

clean:
	rm -f $(PROGS) *.o core
