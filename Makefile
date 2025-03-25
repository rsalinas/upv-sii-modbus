.PHONY: all clean

all: p3-master-cli-alu-v1.zip p3-slave-cli-alu-v1.zip

p3-master-cli-alu-v1.zip: p3-master-cli-alu
	zip $@ -9r p3-master-cli-alu

p3-slave-cli-alu-v1.zip: p3-slave-cli-alu
	zip $@ -9r p3-slave-cli-alu

clean:
	rm -fv p3-master-cli-alu-v1.zip p3-slave-cli-alu-v1.zip
