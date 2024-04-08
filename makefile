.PHONY:all
all:
	@cd comilper_server;\
	make;\
	cd -;\
	cd oj_server;\
	make;\
	cd -; 



.PHONY:output
output:
	mkdir -p output/comilper_server;\
	mkdir -p output/oj_server;\
	cp -rf comilper_server/comilper output/comilper_server;
	cp -rf comilper_server/temp output/comilper_server;

	cp -rf oj_server/conf output/oj_server/
	cp -rf oj_server/oj_server output/oj_server/
	cp -rf oj_server/wwwroot output/oj_server/
	cp -rf oj_server/template_html output/oj_server/
	cp -rf oj_server/questions output/oj_server/

.PHONY:clean
clean:
	@cd comilper_server;\
	make clean;\
	cd -;\
	cd oj_server;\
	make clean;\
	cd -;\
	rm -rf output;