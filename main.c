#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#include <sys/stat.h>

#include <string.h>
#include <errno.h>
#include <unistd.h>
#include <dirent.h>

#include "cJSON.h"

struct stat st = {0};



const char *read_file(const char *path) {
  	FILE *file = fopen(path, "r");
  	if (file == NULL) {
    	return NULL;
  	}
  	fseek(file, 0, SEEK_END);
  	long len = ftell(file);
  	fseek(file, 0, SEEK_SET);
  	char *buffer = malloc(len + 1);

  	if (buffer == NULL) {
    	fprintf(stderr, "Unable to allocate memory for file %s\n", path);
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
		sprintf(cmd, "wget -q https://raw.githubusercontent.com/TheHerowither/WoWPM/master/db.json -O %s/db.json", path);
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

		char addon_path[1041] = {0};
		sprintf(addon_path, "%s/Interface/AddOns", argv[2]);

		char lcfg_path[1024] = {0};
		sprintf(lcfg_path, "%s/wpm.json", argv[2]);

		printf("Checking for manually installed AddOns\n");
		cJSON *addons = cJSON_CreateArray();
		if (addons == NULL) {return 1;}

		DIR *d;
	  	struct dirent *dir;
	  	d = opendir(addon_path);
	  	if (d) {
    		while ((dir = readdir(d)) != NULL) {
				if (dir->d_name[0] != '.') {
					cJSON_AddItemToArray(addons, cJSON_CreateString(dir->d_name));
				}
    		}
	    	closedir(d);
  		}
		cJSON *json = cJSON_CreateObject();
		cJSON_AddItemToObject(json, "addons", addons);

		FILE *local_cfg = fopen(lcfg_path, "w");
		char *json_str = cJSON_Print(json);
		fwrite(json_str, strlen(json_str), 1, local_cfg);
		free(json_str);
		fclose(local_cfg);

		cJSON_Delete(addons);
	}

	if (strcmp(argv[1], "refresh") == 0) {
		char cmd[2067] = {0};
		sprintf(cmd, "wget -q https://raw.githubusercontent.com/TheHerowither/WoWPM/master/db.json -O %s/db.json", path);
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
		if (instance_path == NULL) {
			printf("Please select a WoW instanece first with wpm select\n");
			return 1;
		}
		
		char addon_path[1041] = {0};
		sprintf(addon_path, "%s/Interface/AddOns", instance_path);

		char lcfg_path[1024] = {0};
		sprintf(lcfg_path, "%s/wpm.json", instance_path);

		const char *db_json = read_file(db_file);
		const char *lcfg_json = read_file(lcfg_path);

		cJSON *db = cJSON_Parse(db_json);
		cJSON *lcfg = cJSON_Parse(lcfg_path);

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

		FILE *lcfg_f = fopen(lcfg_path, "w");
		cJSON *addons = cJSON_GetObjectItemCaseSensitive(lcfg, "addons");
		cJSON_AddItemToArray(addons, cJSON_CreateString(argv[2]));
		cJSON_AddItemToObject(lcfg, "addons", addons);
		char *lcfg_str = cJSON_Print(lcfg);
		fwrite(lcfg_str, strlen(lcfg_str), 1, lcfg_f);
		free(lcfg_str);
		fclose(lcfg_f);

		cJSON_Delete(db);
		free((void*)instance_path);
		free((void*)db_json);
		free((void*)lcfg_json);
	}

	if (strcmp(argv[1], "list") == 0) {
		char db_file[1032] = {0};
		sprintf(db_file, "%s/db.json", path);

		const char *db_json = read_file(db_file);

		cJSON *db = cJSON_Parse(db_json);

		cJSON *current_element = NULL;
		char *current_key = NULL;

		cJSON_ArrayForEach(current_element, db) {
    		current_key = current_element->string;
			cJSON *descr = cJSON_GetObjectItemCaseSensitive(current_element, "descr");
    		if (current_key != NULL) {
        		printf("%s - %s\n", current_key, descr->valuestring);
    		}
		}

		free((void *)db_json);
		cJSON_Delete(db);
	}

	if (strcmp(argv[1], "installed") == 0) {
		char inst_file[2048] = {0};
		sprintf(inst_file, "%s/instance", path);

		const char *instance_path = read_file(inst_file);
		char lcfg_path[1024];
		sprintf(lcfg_path, "%s/wpm.json", instance_path);

		const char *lcfg_json = read_file(lcfg_path);

		cJSON *lcfg = cJSON_Parse(lcfg_json);

		cJSON *current_element = NULL;
		char *current_key = NULL;

		cJSON *arr = cJSON_GetObjectItemCaseSensitive(lcfg, "addons");

		cJSON_ArrayForEach(current_element, arr) {
    		current_key = current_element->valuestring;
    		if (current_key != NULL) {
        		printf("%s\n", current_key);
    		}
		}

		free((void*)instance_path);
		free((void*)lcfg_json);
		cJSON_Delete(lcfg);
	}


	return 0;
}
