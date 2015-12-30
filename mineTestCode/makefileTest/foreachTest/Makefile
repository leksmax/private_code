OBJS = $(shell find ./ -name "*.c")
DIROBJS = $(dir $(OBJS))
SORT_OBJS = $(sort $(DIROBJS))
FOREACH_OBJS = $(foreach var, $(dir $(OBJS)), $(var)111)


.PHONY: all
all:
	@echo $(OBJS)
	@echo $(DIROBJS)
	@echo $(SORT_OBJS)
	@echo $(FOREACH_OBJS)
	