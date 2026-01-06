# 定义子目录
SUBDIRS = comm firm_report get_svn_firmware prodReportMonth

# 默认目标
all: $(SUBDIRS)

# 进入每个子目录并调用其 Makefile
$(SUBDIRS):
	$(MAKE) -C $@

# 清理目标
clean:
	for dir in $(SUBDIRS); do \
		$(MAKE) -C $$dir clean; \
	done

.PHONY: all clean $(SUBDIRS)

