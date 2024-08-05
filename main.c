#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>

#include "cJSON.h"

struct stat st = {0};



const char *read_file(const char *path) {
  	FILE *file = fopen(path, "r");
  	if (file == NULL) {
    	fprintf(stderr, "Expected file \"%s\" not found", path);
    	return NULL;
  	}
  	fseek(file, 0, SEEK_END);
  	long len = ftell(file);
  	fseek(file, 0, SEEK_SET);
  	char *buffer = malloc(len + 1);

  	if (buffer == NULL) {
    	fprintf(stderr, "Unable to allocate memory for file");
    	fclose(file);
    	return NULL;
  	}

  	fread(buffer, 1, len, file);
 	buffer[len] = '\0';

  	return (const char *)buffer;
}


int main(int argc, char **argv) {
	char uname[64] = {0};
	getlogin_r(uname, 64);

	char path[1024] = {0};
	sprintf(path, "/home/%s/.wowpm", uname);

	char cwd[1024];
   	if (getcwd(cwd, sizeof(cwd)) != NULL) {
   	} else {
    	perror("getcwd() error");
    	return 1;
   	}
	if (stat(path, &st) == -1) {
		if(mkdir(path, 0777) == -1) {
			printf("[ERROR]: mkdir failed %s\n", strerror(errno));
			return 1;
		}
		char cmd[2067] = {0};
		sprintf(cmd, "cp %s/db.json %s/db.json", cwd, path);
		printf("[CMD]: %s\n", cmd);
		system(cmd);
	}

	if (argc < 2) {
		printf("Invalid usage!\n");
		printf("Usage:\n	%s <mode> <args>", argv[0]);
		return 1;
	}

	if (strcmp(argv[1], "select") == 0) {
		if (argc < 3) {
			printf("[ERROR]: select mode requires one argument, <path>\n");
			return 1;
		}

		char inst_file[1033] = {0};
		sprintf(inst_file, "%s/instance", path);

		FILE *cfg = fopen(inst_file, "w");
		fwrite(argv[2], strlen(argv[2]), 1, cfg);
		fclose(cfg);
	}

	if (strcmp(argv[1], "refresh") == 0) {
		char cmd[2067] = {0};
		sprintf(cmd, "cp %s/db.json %s/db.json", cwd, path);
		printf("[CMD]: %s\n", cmd);
		system(cmd);
	}

	if (strcmp(argv[1], "install") == 0) {
		if (argc < 3) {
			printf("[ERROR]: install mode requires one argument, <addon>\n");
			return 1;
		}

		char inst_file[1033] = {0};
		sprintf(inst_file, "%s/instance", path);

		char db_file[1032] = {0};
		sprintf(db_file, "%s/db.json", path);

		const char *instance_path = read_file(inst_file);
		
		char addon_path[1041] = {0};
		sprintf(addon_path, "%s/Interface/AddOns", instance_path);

		const char *db_json = read_file(db_file);
		cJSON *db = cJSON_Parse(db_json);

		if (db == NULL) { 
        	const char *error_ptr = cJSON_GetErrorPtr(); 
        	if (error_ptr != NULL) { 
            	printf("Error: %s\n", error_ptr); 
        	} 
        	cJSON_Delete(db); 
        	return 1; 
    	}
		
		cJSON *addon = cJSON_GetObjectItemCaseSensitive(db, argv[2]);
		if (addon == NULL) { 
        	const char *error_ptr = cJSON_GetErrorPtr(); 
        	if (error_ptr != NULL) { 
            	printf("Error: %s\n", error_ptr); 
        	} 
        	cJSON_Delete(db);
			printf("Addon %s is not in %s\n", argv[2], db_file);
        	return 1; 
    	}
		cJSON *addon_url = cJSON_GetObjectItemCaseSensitive(addon, "url");
		cJSON *addon_rename = cJSON_GetObjectItemCaseSensitive(addon, "rename");

		if (cJSON_IsString(addon_url) && (addon_url->valuestring != NULL)) { 
        	char cmd[2094] = {0};
			sprintf(cmd, "wget -q %s -O %s/%s.zip", addon_url->valuestring, path, argv[2]);
			printf("[CMD]: %s\n", cmd);
			system(cmd);
			sprintf(cmd, "unzip -o -qq %s/%s.zip -d %s", path, argv[2], addon_path);
			printf("[CMD]: %s\n", cmd);
			system(cmd);

			if (cJSON_IsTrue(addon_rename)) {
				sprintf(cmd, "mv %s/%s-master %s/%s", addon_path, argv[2], addon_path, argv[2]);
				printf("[CMD]: %s\n", cmd);
				system(cmd);
			}
    	}

		cJSON_Delete(db);
		free((void*)instance_path);
		free((void*)db_json);
	}

	return 0;
}
