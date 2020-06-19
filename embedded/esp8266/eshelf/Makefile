upload:
	# uploads the compiled bin firmware to the board over wifi
	@username=$$(cat ota_config.h | grep 'update_username' | cut -f3 -d ' ' | tr -d '\"');\
	password=$$(cat ota_config.h | grep 'update_password' | cut -f3 -d ' ' | tr -d '\"');\
	host=$$(cat mdns_config.h | grep 'host' | cut -f3 -d ' ' | tr -d '\"');\
	path=$$(cat ota_config.h | grep 'update_path' | cut -f3 -d ' ' | tr -d '\"');\
	curl -u $$username:$$password -F 'image=@bin/eshelf.ino.bin' https://$$host.local$$path --insecure;

ping:
	# pings the board to see if it's online a ready to comunicate
	@host=$$(cat mdns_config.h | grep 'host' | cut -f3 -d ' ' | tr -d '\"');\
	ping -c 2 $$host.local;

gen-docs:
	# generates the documentation for the C/C++ code
	@doxygen eshelf.ino

gen-certs:
	# generates the tls certificates requiered by the https protocol
	# heavily modified script from the official ESP8266 library
	@BITS=1024;\
	C=$$PWD;\
	cd certificates;\
	mkdir -p tmp;\
	cd tmp;\
	openssl genrsa -out tls.ca_key.pem $$BITS;\
	openssl genrsa -out tls.key_$$BITS.pem $$BITS;\
	openssl rsa -in tls.key_$$BITS.pem -out tls.key_$$BITS -outform DER;\
	echo "[ req ]\ndistinguished_name = req_distinguished_name\nprompt = no\n\n[ req_distinguished_name ]\nO = eshelf\nCN = 127.0.0.1" > certs.conf;\
	openssl req -out tls.ca_x509.req -key tls.ca_key.pem -new -config certs.conf;\
	openssl req -out tls.x509_$$BITS.req -key tls.key_$$BITS.pem -new -config certs.conf;\
	openssl x509 -req -in tls.ca_x509.req  -out tls.ca_x509.pem -sha256 -days 5000 -signkey tls.ca_key.pem;\
	openssl x509 -req -in tls.x509_$$BITS.req  -out tls.x509_$$BITS.pem -sha256 -CAcreateserial -days 5000 -CA tls.ca_x509.pem -CAkey tls.ca_key.pem;\
	openssl x509 -in tls.ca_x509.pem -outform DER -out tls.ca_x509.cer;\
	openssl x509 -in tls.x509_$$BITS.pem -outform DER -out tls.x509_$$BITS.cer;\
	xxd -i tls.key_$$BITS       | sed 's/.*{//' | sed 's/\};//' | sed 's/unsigned.*//' > "../key.h";\
	xxd -i tls.x509_$$BITS.cer  | sed 's/.*{//' | sed 's/\};//' | sed 's/unsigned.*//' > "../x509.h";\
	rm -f tls.ca_key.pem tls.key_$$BITS.pem tls.key_$$BITS certs.conf tls.ca_x509.req tls.x509_$$BITS.req tls.ca_x509.pem tls.x509_$$BITS.pem tls.srl tls.x509_$$BITS.cer tls.ca_x509.cer;\
	cd ..;\
	rmdir tmp;

clean:
	# cleans the compiled stuff
	@rm -rf bin;\
	rm -f certificates/key.h;\
	rm -f certificates/x509.h;
