IMAGE=task_7_image
CONTAINER=task
MOUNT_PATH=/home/
SHELL=/bin/bash
RUN_COMMAND="cd /home; cmake .; make; ./program"
CLEAN_COMMAND="cd /home; rm -r CMakeFiles CMakeCache.txt cmake_install.cmake Makefile  output.mkv  program"

image:
	docker build -t $(IMAGE) . 
run:
	@if docker ps -a | grep $(CONTAINER) 1>/dev/null; then \
		docker stop $(CONTAINER) 1>/dev/null; \
		docker rm $(CONTAINER) 1>/dev/null; \
	fi
	docker run -tid --name="$(CONTAINER)" -v $(PWD)/..:$(MOUNT_PATH) $(IMAGE) $(SHELL)
	docker exec -id $(CONTAINER) $(SHELL) -c $(RUN_COMMAND)
sh:
	docker start $(CONTAINER) 
	docker attach $(CONTAINER)
clean:
	docker exec -id $(CONTAINER) $(SHELL) -c $(CLEAN_COMMAND)
rm:
	docker stop $(CONTAINER)
	docker rm $(CONTAINER)
