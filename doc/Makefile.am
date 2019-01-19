#ACLOCAL_AMFLAGS = -I ${top_srcdir}/m4

EXTRA_DIST = doxygen/doxygen.cfg

MOSTLYCLEANFILES = $(DX_CLEANFILES)

# Documentation hooks in with the 'html' target
html-local: doxygen-run

# Target for all documentation
docs: html-local

# if DX_COND_doc
# install-html: docs-local
# 	$(MKDIR_P) "$(DESTDIR)$(docdir)/html"
# 	find docs/html -type d -exec $(INSTALL) -d {,$(DESTDIR)$(docdir)/html/}{} \;
# else
# install-html:

# endif

install-html-am: docs
	$(MKDIR_P) "$(DESTDIR)$(docdir)/html"
	cp -r docs/html/* $(DESTDIR)$(docdir)/html/

uninstall-html:
	rm -rf $(DESTDIR)$(docdir)/html

uninstall-local: uninstall-html

include doxygen/doxygen.am
