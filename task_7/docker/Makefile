IMAGE=task_7_image
CONTAINER=task
MOUNT_PATH=/home/
SHELL=/bin/bash
COMMAND="cd /home; make; ./program"

image:
	docker build -t $(IMAGE) . 
run:
	@if docker ps -a | grep $(CONTAINER) 1>/dev/null; then \
		docker stop $(CONTAINER) 1>/dev/null; \
		docker rm $(CONTAINER) 1>/dev/null; \
	fi
	docker run -tid --name="$(CONTAINER)" -v $(PWD)/..:$(MOUNT_PATH) $(IMAGE) $(SHELL)
	docker exec -id $(CONTAINER) $(SHELL) -c $(COMMAND)
sh:
	docker start $(CONTAINER) 
	docker attach $(CONTAINER)
rm:
	docker stop $(CONTAINER)
	docker rm $(CONTAINER)
