quiet-command = $(if ${V},${1},$(if ${2},@echo ${2} && ${1}, @${1}))

MANPAGES += bee
MANPAGES += bee-check
MANPAGES += bee-download
MANPAGES += bee-list
MANPAGES += bee-init
MANPAGES += bee-install
MANPAGES += bee-query
MANPAGES += bee-remove

XMLTO       = xmlto
XMLTO_FLAGS = --stringparam man.output.quietly=1

.PHONY: all
all:
	@echo "Following targets are availabe:"
	@echo "        xml2man - update manpages"

.PHONY: xml2man
xml2man: force-clean $(addsuffix .1.in,$(MANPAGES))

.PHONY: force-clean
force-clean:
	$(call quiet-command,rm -f $(addsuffix .1.in,$(MANPAGES)),"CLEAN	*.1.in")
	$(call quiet-command,rm -f $(addsuffix .1,   $(MANPAGES)),"CLEAN	*.1")

%.1: %.xml
	$(call quiet-command,$(XMLTO) $(XMLTO_FLAGS) man $<,"XMLTO	$<")

%.1.in: %.1
	$(call quiet-command,mv $< $@,"RENAME	$<")
