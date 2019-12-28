#define _CRT_SECURE_NO_WARNINGS
#include<iostream>
#include<string>
#include<vector>
#include<sstream>
extern "C" {
	#include<cstring>
	#include<stdlib.h>
};

using namespace std;

#define ROOT_START 19*512
#define DATA_START 33*512
#define SIMPLE_MODE 0
#define COMPLEX_MODE 1

struct DIR {
	char name[8];	//�ļ���
	char extension[3];	//�ļ���չ��
	char attr;	//�ļ�����
	char reserved;
	char creation_time;
	char created_time[2];
	char created_date[2];
	char last_accessed_date[2];
	char cluster_num[2];	
	char last_modify_time[2];
	char last_modify_date[2];
	short int first_cluster;	//Entry�ĵ�һ���غ�
	int size;	//�ļ���С
};

vector<string> split(string, char);//�ַ����ָ�
void comd_process(vector<string>);//ָ���
void get_start_position(string);//�õ�ָ��·������ʼ��ַ
int get_position(vector<string>, int, int, bool);
void list_contents(string path, int start, DIR* dir, int mode);//�о�path�µ��ļ���Ŀ¼
string to_red(string s, int attr);
bool is_empty_dir(DIR*);//�ж�һ��Ŀ¼���Ƿ�Ϊ��
string get_filename(char * name, char * extension, int type);//���·�����ļ����У����ļ���
void get_file_content(int start,string file_name);//��ȡ�ļ�����
bool is_valid_l(string);

void my_print(string, int);
extern "C" {
	void nasm_print(char*, int);
};

FILE* file = fopen("./a.img", "r");

extern int start_loc = ROOT_START;	//��ʾ����·����ʼ��ַ

int main() {
	string command;	
	string tip = "Please input your command(input 'exit' to exit):";
	while (true) {
		my_print(tip, tip.length());
		getline(cin, command);
		vector<string> comd_split = split(command, ' ');
		comd_process(comd_split);
		start_loc = ROOT_START;
	}
	system("pause");
	return 0;
}

//�ַ����ָ�
vector<string> split(string s, char sep) {
	vector<string> result;
	stringstream sstream(s);
	string temp;
	while (getline(sstream, temp, sep)) {
		result.push_back(temp);
	}
	return result;
}
//ָ���
void comd_process(vector<string> comd) {
	string path = "/";//Ĭ��·���Ǹ�Ŀ¼
	int comd_size = comd.size();
	int path_count = 0;
	bool is_valid = true;
	string error_msg;
	if (comd[0] == "ls") {	
		int mode = SIMPLE_MODE;	//ls��ģʽ,Ĭ�ϼ�ģʽ
		bool is_print_num = false;
		if (comd_size == 2) {
			if (comd[1].substr(0, 1) == "/") {
				if (comd[1].find(".") == -1) {
					path = comd[1];
				}
				else {
					is_valid = false;
					error_msg = "ls doesn't support file!";
				}
			}
			else if(comd[1].substr(0, 2) == "-l"){				
				is_valid = is_valid_l(comd[1].substr(2, comd[1].length() - 2));
				if (is_valid == false) {
					error_msg = "Invalid Command!";
				}
				else {
					path = "/";
					mode = COMPLEX_MODE;
				}
			}
			else {
				error_msg = "Invalid Command!";
				is_valid = false;
			}
		}
		else {
			for (int i = 1; i < comd_size; i++) {
				if (comd[i].substr(0, 1) == "/") {
					if (comd[i].find(".") == -1) {
						path = comd[i];
						path_count++;
					}
					else {
						is_valid = false;
						error_msg = "ls doesn't support file!";
						break;
					}
					
				}
				else if (comd[i].substr(0, 2) == "-l") {
					mode = COMPLEX_MODE;
					is_valid = is_valid_l(comd[i].substr(2, comd[i].length() - 2));
					if (is_valid == false) {
						error_msg = "Invalid Command!";
					}
				}else {
					error_msg = "Invalid Command!";
					is_valid = false;
				}
			}
			if (path_count > 1) {
				error_msg = "Too many paths!";
				is_valid = false;
			}			
		}
		if (is_valid) {
			//����ls��غ���
			get_start_position(path);
			if (start_loc == -1) {
				error_msg = "Invalid Path!";
				is_valid = false;
				my_print(error_msg, error_msg.length());
			}
			else {
				DIR* dir = new DIR;
				if (path.substr(path.length() - 1, 1) != "/") {
					path += "/";
				}
				list_contents(path, start_loc, dir, mode);
				delete dir;
			}
		}
		else {
			my_print(error_msg, error_msg.length());
		}
	}
	else if (comd[0] == "cat") {
		string sup_path;//�洢�ļ����ϼ�Ŀ¼���Ա��ѯ�ļ��Ĵ�С
		if (comd.size() != 2) {
			is_valid = false;
			error_msg = "Too many parameter!";
		}
		else {
			if (comd[1].find(".") == -1) {
				is_valid = false;
				error_msg = "Invalid parameter!";
			}
			else {
				if (comd[1].substr(0, 1) != "/") {
					path = path + comd[1];
				}
				else {
					path = comd[1];
				}
			}
		}
		if (is_valid) {
			int index = path.find_last_of("/");
			sup_path = path.substr(0, index + 1);
			string file_name = path.substr(index + 1);
			get_start_position(sup_path);
			int sup_start = start_loc;
			//get_start_position(path);
			if (start_loc == -1) {
				error_msg = "Invalid filepath!";
				my_print(error_msg, error_msg.length());
			}
			else {
				get_file_content(sup_start, file_name);
			}
		}
		else {
			my_print(error_msg, error_msg.length());
		}
		
	}
	else if (comd[0] == "exit") {
		exit(0);
	}
	else {
		error_msg = "Invalid command!";
		my_print(error_msg, error_msg.length());
	}
}

bool is_valid_l(string s) {
	bool is_valid = true;
	int length = s.length();
	for (int i = 0; i < length; i++) {
		if (s.substr(i, 1) != "l") {
			is_valid = false;
			break;
		}
	}
	return is_valid;
}

void get_start_position(string path) {
	vector<string> path_split = split(path, '/');
	int deepth = path_split.size();
	if (path == "/") {
		start_loc = ROOT_START;
	}
	else {
		get_position(path_split, deepth, 1, false);
	}
}

int get_position(vector<string> path_split, int deepth, int finished_deepth, bool is_get) {
	if (finished_deepth == deepth) {
		if (is_get) {
			return start_loc;
		}
		else {
			start_loc = -1;
			return -1;
		}
	}
	else {
		fseek(file, start_loc, SEEK_SET);
		DIR *dir = new DIR;
		fread(dir, sizeof(DIR), 1, file);
		while (!is_empty_dir(dir)) {
			string file_name = get_filename(dir->name, dir->extension, dir->attr);
			if (file_name == path_split[finished_deepth]) {
				is_get = true;
				finished_deepth++;
				start_loc = (dir->first_cluster - 2) * 512 + DATA_START;
				get_position(path_split, deepth, finished_deepth, is_get);
				break;
			}
			else {
				is_get = false;
				start_loc = -1;
				fread(dir, sizeof(DIR), 1, file);				
			}
		}
		delete dir;
	}
}

void list_contents(string path, int start, DIR* dir, int mode) {
	string output;
	string path_backup = path;
	int start_backup = start;
	int directory_num = 0;
	int file_num = 0;
	vector<string> items;//�洢��·���µ�Ŀ¼���ļ���
	vector<DIR> DIRs;
	fseek(file, start, SEEK_SET);
	fread(dir, sizeof(DIR), 1, file);
	while (!is_empty_dir(dir)) {
		DIRs.push_back(*dir);
		char* name = dir->name;
		char* extension = dir->extension;
		int type = dir->attr;
		if (type == 0x10) {
			if (*name!='.') {
				directory_num++;
			}
		}
		else {
			file_num++;
		}
		string file_name = get_filename(name, extension, type);
		items.push_back(file_name);
		//fseek(file, 0, SEEK_CUR);
		fread(dir, sizeof(DIR), 1, file);
	}
	if (mode == 1) {
		output = path + "  " + to_string(directory_num) + " " + to_string(file_num) + ":";
		my_print(output, output.length());
	}
	else {
		output = path + ":";
		my_print(output,output.length());
	}
	directory_num = 0;
	file_num = 0;
	string subs = "";
	for (int i = 0; i < items.size(); i++) {
		if (mode == 0) {
			if (i != items.size() - 1) {
				subs += to_red(items[i], DIRs[i].attr) + "  ";
			}
			else {
				subs += to_red(items[i], DIRs[i].attr);
				my_print(subs, subs.length());
			}
		}
		else {
			int offset = (DIRs[i].first_cluster - 2) * 512 + DATA_START;
			fseek(file, offset, SEEK_SET);
			DIR* sub_dir = new DIR;
			fread(sub_dir, sizeof(DIR), 1, file);
			//ֱ���ļ���
			if (DIRs[i].attr == 0x10 && items[i] != "." && items[i] != "..") {								
				while (!is_empty_dir(sub_dir)) {
					char* name = sub_dir->name;
					char* extension = sub_dir->extension;
					int type = sub_dir->attr;
					if (type == 0x10) {
						if (*name!='.') {
							directory_num++;
						}
					}
					else {
						file_num++;
					}
					fread(sub_dir, sizeof(DIR), 1, file);
				}
				subs = to_red(items[i],0x10) + "  " + to_string(directory_num) + " " + to_string(file_num);
				my_print(subs, subs.length());
				subs = "";
			}
			//�ı��ļ�
			else if (items[i] == "." || items[i] == "..") {
				my_print(to_red(items[i], 0x10) , items[i].length());
			}else{
				int size = DIRs[i].size;
				subs = items[i] + "  " + to_string(size);
				my_print(subs, subs.length());
				subs = "";
				//cout << items[i] + "  " + to_string(size) << endl;
			}
			directory_num = 0;
			file_num = 0;
			delete sub_dir;
		}
	}
	fseek(file, start_backup, SEEK_SET);
	for (int i = 0; i < DIRs.size(); i++) {
		if (DIRs[i].attr == 0x10 && items[i] != "." && items[i] != "..") {
			path = path + items[i] + "/";
			int offset = (DIRs[i].first_cluster - 2) * 512 + DATA_START;
			DIR* sub_dir = new DIR;
			fread(sub_dir, sizeof(DIR), 1, file);
			list_contents(path, offset, sub_dir, mode);
			delete sub_dir;
		}
		path = path_backup;
		//fseek(file, start_backup, SEEK_SET);
	}
}

string to_red(string s, int attr) {
	if (attr == 0x10) {
		return "\033[1;31m" + s + "\033[0m";
	}
	else {
		return s;
	}
}
void get_file_content(int start,string file_name) {
	DIR* sup_dir = new DIR;
	fseek(file, start, SEEK_SET);
	fread(sup_dir, sizeof(DIR), 1, file);
	bool is_found = false;
	while (!is_empty_dir(sup_dir)) {
		string this_name = get_filename(sup_dir->name, sup_dir->extension, sup_dir->attr);
		if (this_name == file_name) {
			is_found = true;
			break;
		}
		else {
			fread(sup_dir, sizeof(DIR), 1, file);
		}
	}
	if (is_found) {
		int offset = (sup_dir->first_cluster - 2) * 512 + DATA_START;
		int size = sup_dir->size;
		fseek(file, offset, SEEK_SET);
		char* txt = new char[size];
		fread(txt, size, 1, file);
		my_print(txt, size);
		//cout << txt << endl;
		delete[] txt;
	}
	else {
		string error = "Invalid filepath!";
		my_print(error, error.length());
		//cout << "Invalid filepath!" << endl;
	}
	delete sup_dir;
}

bool is_empty_dir(DIR* dir) {
	bool is_empty = true;
	char* p = (char *)(void*)dir;
	for (int i = 0; i < 32; i++) {
		if (*p > 0) {
			is_empty = false;
			break;
		}
		p++;
	}
	return is_empty;
}

string get_filename(char * name, char * extension, int type) {
	string filename = "";
	for (int i = 0; i < 8; i++) {
		if (name[i] != ' ') {
			filename += name[i];
		}
		else {
			break;
		}
	}
	if (type != 0x10) {
		filename = filename + '.' + extension;
	}
	return filename;
}

void my_print(string msg, int length) {
	msg.append("\n");
	cout << msg;
	char *p = new char[msg.length() + 1];
	strcpy(p, msg.c_str());
	//nasm_print(p, length + 2);
}
