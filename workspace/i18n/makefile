.PHONY: tr-audit tr-keys

# Run from repo root:
#   make -f workspace/i18n/makefile tr-audit

ROOT ?= .
LANG ?= workspace/i18n/locales/zh_CN.lang

EXCLUDE ?= build/,workspace/_unmaintained/,workspace/**/cores/patches/,skeleton/_unmaintained/

tr-audit:
	python workspace/i18n/tr_audit.py --root $(ROOT) --lang $(LANG) --exclude "$(EXCLUDE)"

tr-keys:
	python workspace/i18n/tr_audit.py --root $(ROOT) --lang $(LANG) --exclude "$(EXCLUDE)" --write-keys workspace/i18n/keys.txt
