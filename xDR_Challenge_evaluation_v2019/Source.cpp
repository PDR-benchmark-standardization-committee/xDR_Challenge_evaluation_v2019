//This script is an evaluation tool for the xDR Challenge 2019 in Industrial Scnario competition.

#include<iostream>
#include<windows.h>
#include<fstream>
#include<vector>
#include<string>
#include<sstream>
#include<omp.h>
#include<algorithm>
#include<numeric>
#include<math.h>
#include<direct.h>
#include<time.h>
#include<chrono>
#include<functional>
#include<opencv2/opencv.hpp>

//Load Data
////Number of conf
#define BASE_DNAME 0
#define MAP_SIZE_FNAME 1
#define OBSTACLE_FNAME 2
#define SENS_START_END_FNAME 3
#define OPTIONAL_DNAME 4

using namespace std;
using namespace cv;

vector<string> split(string& input, char delimiter) {
	istringstream stream(input);
	string field;
	vector<string> result;
	while (getline(stream, field, delimiter)) {
		result.push_back(field);
	}
	return result;
}

//Laod start----------------------------------------------------------------------------------------------------------
////Load config.ini file
vector<string> Config(string selected_track) {
	cout << "Loading configuration file." << endl;
	vector<string> conf;
	char buf[1024];
	GetPrivateProfileString("base", "base_dname", "no data", buf, sizeof(buf), ".\\config.ini");
	conf.push_back(string(buf) + selected_track);
	GetPrivateProfileString("base", "map_size_fname", "no data", buf, sizeof(buf), ".\\config.ini");
	conf.push_back(string(buf));
	GetPrivateProfileString("base", "obstacle_fname", "no data", buf, sizeof(buf), ".\\config.ini");
	conf.push_back(string(buf));
	GetPrivateProfileString("base", "sens_start_end_fname", "no data", buf, sizeof(buf), ".\\config.ini");
	conf.push_back(string(buf));
	GetPrivateProfileString("optinal", "optional_dname", "no data", buf, sizeof(buf), ".\\config.ini");
	conf.push_back(string(buf) + selected_track);
	GetPrivateProfileString("submitted", "submitted_dname", "no data", buf, sizeof(buf), ".\\config.ini");
	string sub_dname = string(buf) + "\\" + selected_track;
	WIN32_FIND_DATA win32fd;
	HANDLE hFind;
	string search_dname_fname = ".\\" + sub_dname + "\\*.*";
	hFind = FindFirstFile(search_dname_fname.c_str(), &win32fd);
	int count = 0;
	if (hFind == INVALID_HANDLE_VALUE) {
		cerr << sub_dname << " not found" << endl;
	}
	else {
		do {
			string sDirName(win32fd.cFileName);
			if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY && sDirName != "." && sDirName != "..") {//except current directory and parent directory
				cout << sDirName << endl;
				conf.push_back(sub_dname + "\\" + sDirName);
				count++;
			}
		} while (FindNextFile(hFind, &win32fd));
		FindClose(hFind);
	}
	
	cout << count << " load the submitted data of the directory." << endl;
	cout << "Configuration file load complete!" << endl;
	return conf;
}

/////////////////////////
////Load trajectory files
vector<vector<string>> Trajectory(string submitted_dname) {
	cout << submitted_dname << " Loading submitted file." << endl;
	WIN32_FIND_DATA win32fd;
	HANDLE hFind;
	vector<vector<string>> data;
	string search_fname = ".\\" + submitted_dname + "\\PDR_Traj_*.*";
	hFind = FindFirstFile(search_fname.c_str(), &win32fd);
	if (hFind == INVALID_HANDLE_VALUE) {
		cerr << submitted_dname << " not found" << endl;
	}
	else {
		do {
			if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			}
			else {
				vector<string> tra_data;
				string sFileName(win32fd.cFileName);
				cout << sFileName << endl;
				tra_data.push_back(sFileName);
				ifstream ifs(".\\" + submitted_dname + "\\" + win32fd.cFileName);
				string buf;
				while (ifs && getline(ifs, buf)) {
					tra_data.push_back(buf);
				}
				data.push_back(tra_data);
			}
		} while (FindNextFile(hFind, &win32fd));
		FindClose(hFind);
	}
	cout << "Trajectory data load complete!" << endl;
	return data;
}

////Load Area files
vector<vector<string>> Area(string submitted_dname) {
	cout << submitted_dname << " Loading submitted file." << endl;
	WIN32_FIND_DATA win32fd;
	HANDLE hFind;
	vector<vector<string>> data;
	string search_fname = ".\\" + submitted_dname + "\\PDR_Area_*.*";
	hFind = FindFirstFile(search_fname.c_str(), &win32fd);
	if (hFind == INVALID_HANDLE_VALUE) {
		cerr << submitted_dname << " not found" << endl;
	}
	else {
		do {
			if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			}
			else {
				vector<string> area_data;
				string sFileName(win32fd.cFileName);
				cout << sFileName << endl;
				area_data.push_back(sFileName);
				ifstream ifs(".\\" + submitted_dname + "\\" + win32fd.cFileName);
				string buf;
				while (ifs && getline(ifs, buf)) {
					area_data.push_back(buf);
				}
				data.push_back(area_data);
			}
		} while (FindNextFile(hFind, &win32fd));
		FindClose(hFind);
	}
	cout << "Area data load complete!" << endl;
	return data;
}

////Load Soe files
vector<vector<string>> Soe(string submitted_dname) {
	cout << submitted_dname << " Loading submitted file." << endl;
	WIN32_FIND_DATA win32fd;
	HANDLE hFind;
	vector<vector<string>> data;
	string search_fname = ".\\" + submitted_dname + "\\Estimated_Operation*.*";
	hFind = FindFirstFile(search_fname.c_str(), &win32fd);
	if (hFind == INVALID_HANDLE_VALUE) {
		cerr << submitted_dname << " not found" << endl;
	}
	else {
		do {
			if (win32fd.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) {
			}
			else {
				vector<string> soe_data;
				string sFileName(win32fd.cFileName);
				cout << sFileName << endl;
				soe_data.push_back(sFileName);
				ifstream ifs(".\\" + submitted_dname + "\\" + win32fd.cFileName);
				string buf;
				while (ifs && getline(ifs, buf)) {
					soe_data.push_back(buf);
				}
				data.push_back(soe_data);
			}
		} while (FindNextFile(hFind, &win32fd));
		FindClose(hFind);
	}
	cout << "Soe data load complete!" << endl;
	return data;
}

//////////////////////////
////Load true points data
vector<string> True_points_data(string base_dname, string trajectory_fname) {
	vector<string> data;
	ifstream ifs(".\\" + base_dname + "\\" + trajectory_fname.erase(trajectory_fname.size() - 4) + "_true_points.csv");
	string buf;
	getline(ifs, buf);	//header skip
	while (ifs && getline(ifs, buf)) {
		data.push_back(buf);
	}
	cout << "True points data load complete!" << endl;
	return data;
}

////Load true pass data
vector<string> True_pass_data(string base_dname, string trajectory_fname) {
	vector<string> data;
	ifstream ifs(".\\" + base_dname + "\\" + trajectory_fname.erase(trajectory_fname.size() - 4) + "_true_pass.csv");
	string buf;
	getline(ifs, buf);	//header skip
	while (ifs && getline(ifs, buf)) {
		data.push_back(buf);
	}
	cout << "True pass data load complete!" << endl;
	return data;
}

////Load bup info data
vector<string> BUP_info_data(string base_dname, string trajectory_fname) {
	vector<string> data;
	ifstream ifs(".\\" + base_dname + "\\" + trajectory_fname.erase(trajectory_fname.size() - 4) + "_bup_info.csv");
	string buf;
	getline(ifs, buf);	//header skip
	while (ifs && getline(ifs, buf)) {
		data.push_back(buf);
	}
	cout << "BUP info data load complete!" << endl;
	return data;
}

////Load map size data
vector<double> Map_size(string base_dname, string map_size_fname) {
	vector<double> data;
	vector<string> string_data;
	ifstream ifs(".\\" + base_dname + "\\" + map_size_fname);
	string buf;
	getline(ifs, buf);	//header skip
	while (ifs && getline(ifs, buf)) {
		string_data = split(buf, ',');
		vector<double> double_data(string_data.size());
		transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
			return stod(val);
			});
		data = double_data;
	}
	cout << "Map size data load complete!" << endl;
	return data;
}

////Load obstacle data
vector<vector<int>> Obstacle_data(string base_dname, string obstacle_fname) {
	vector<vector<int>> data;
	Mat map_img = imread(".\\" + base_dname + "\\" + obstacle_fname);
	if (map_img.empty()) return data;
	int obstacle_flag;
	int y_max = map_img.size[0];
	int x_max = map_img.size[1];
	for (int y = 0; y < y_max; y++) {
		vector<int> obsltacle_line;
		for (int x = 0; x < x_max; x++) {
			Vec3b* src = map_img.ptr<Vec3b>(y);
			if (src[x][0] == 255) obstacle_flag = 0;
			else obstacle_flag = 1;
			obsltacle_line.push_back(obstacle_flag);
		}
		data.push_back(obsltacle_line);
	}
	cout << "Obstacle data load complete!" << endl;
	return data;
}

////Load sens start end data
vector<vector<string>> Sens_start_end_data(string base_dname, string sens_start_end_fname) {
	vector<vector<string>> data;
	vector<string> string_data;
	ifstream ifs(".\\" + base_dname + "\\" + sens_start_end_fname);
	string buf;
	getline(ifs, buf);	//header skip
	while (ifs && getline(ifs, buf)) {
		string_data = split(buf, ',');
		data.push_back(string_data);
	}
	cout << "Sensor start end data load complete!" << endl;
	return data;
}

////Load area division data
vector<string> Area_division_data(string optinal_dname, string area_fname) {
	vector<string> data;
	ifstream ifs(".\\" + optinal_dname + "\\AreaGT_" + area_fname.erase(area_fname.size() - 4) + ".csv");
	string buf;
	getline(ifs, buf);	//header skip
	while (ifs && getline(ifs, buf)) {
		data.push_back(buf);
	}
	cout << "Area division data load complete!" << endl;
	return data;
}

////Load soe distinction data
vector<string> Soe_distinction_data(string optinal_dname, string soe_fname) {
	vector<string> data;
	ifstream ifs(".\\" + optinal_dname + "\\" + soe_fname.erase(soe_fname.size() - 4) + "_true_data.csv");
	string buf;
	getline(ifs, buf);	//header skip
	while (ifs && getline(ifs, buf)) {
		data.push_back(buf);
	}
	cout << "Soe distinction data load complete!" << endl;
	return data;
}
//Laod end------------------------------------------------------------------------------------------------------------


//Calculate scare start----------------------------------------------------------------------------------------------- 

vector<double> E_median_error(vector<string> tra_data, vector<string> true_points, vector<string> true_pass, vector<string> bup_info) {
	cout << "Calculat madean error.." << endl;
	vector<double> score_eCDF;
	double score = 0.0;
	vector<vector<double>> true_pass_eval;
	vector<double> true_pass_t;
	vector<double> true_pass_x;
	vector<double> true_pass_y;

	vector<double> bup_start;
	vector<double> bup_end;
	for (int i = 0; i < bup_info.size(); i++) {
		vector<string> string_data = split(bup_info[i], ',');
		vector<double> double_data(string_data.size());
		transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
			return stod(val);
			});
		bup_start.push_back(double_data[0]);
		bup_end.push_back(double_data[1]);
	}

	for (int i = 0; i < true_pass.size(); i++) {
		auto itr = find(true_points.begin(), true_points.end(), true_pass[i]);
		if (itr == true_points.end()) {
			vector<string> string_data = split(true_pass[i], ',');
			vector<double> double_data(string_data.size());
			transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
				return stod(val);
				});

			bool bup_flag = false;
			for (int j = 0; j < bup_start.size(); j++) {
				if (bup_start[j] <= double_data[0] && double_data[0] <= bup_end[j]) {
					bup_flag = true;
				}
			}
			if (!bup_flag) {
				true_pass_t.push_back(double_data[0]);
				true_pass_x.push_back(double_data[1]);
				true_pass_y.push_back(double_data[2]);
			}
		}
	}
	true_pass_eval.push_back(true_pass_t);
	true_pass_eval.push_back(true_pass_x);
	true_pass_eval.push_back(true_pass_y);

	vector<double> error_m_list;
	int cal_count = 0;
	for (int i = 0; i < true_pass_eval[0].size(); i++) {
		int true_timestamp = true_pass_eval[0][i];
		for (int j = 1; j < tra_data.size(); j++) {
			vector<string> string_data = split(tra_data[j], ',');
			vector<double> double_data(string_data.size());
			transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
				return stod(val);
				});
			int tra_timestamp = double_data[0];
			if (true_timestamp == tra_timestamp) {
				double error_m = hypot(true_pass_eval[1][i] - double_data[1], true_pass_eval[2][i] - double_data[2]);
				error_m_list.push_back(error_m);
				cal_count++;
				break;
			}
		}
	}
	sort(error_m_list.begin(), error_m_list.end());
	double error_median = error_m_list[error_m_list.size() / 2];
	double score_cal = -(100.0 / 29.0) * error_median + (3000.0 / 29.0);
	score = score_cal * (cal_count / true_pass_eval[0].size());
	if (score > 100.0) {
		score = 100.0;
	}

	score_eCDF.push_back(score);
	for (int i = 0; i < error_m_list.size(); i++) {
		score_eCDF.push_back(error_m_list[i]);
	}

	return score_eCDF;
}

vector<double> E_accum_error(vector<string> tra_data, vector<string> true_points, vector<string> true_pass, vector<string> bup_info) {
	cout << "Calculat accumulating error.." << endl;
	vector<double> score_eCDF;
	double score = 0.0;
	vector<vector<double>> true_pass_eval;
	vector<double> true_pass_t;
	vector<double> true_pass_x;
	vector<double> true_pass_y;
	vector<double> true_pass_d;

	vector<double> bup_start;
	vector<double> bup_end;
	for (int i = 0; i < bup_info.size(); i++) {
		vector<string> string_data = split(bup_info[i], ',');
		vector<double> double_data(string_data.size());
		transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
			return stod(val);
			});
		bup_start.push_back(double_data[0]);
		bup_end.push_back(double_data[1]);
	}

	for (int i = 0; i < true_pass.size(); i++) {
		auto itr = find(true_points.begin(), true_points.end(), true_pass[i]);
		if (itr == true_points.end()) {
			vector<string> string_data = split(true_pass[i], ',');
			vector<double> double_data(string_data.size());
			transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
				return stod(val);
				});
			bool bup_flag = false;
			for (int j = 0; j < bup_start.size(); j++) {
				if (bup_start[j] <= double_data[0] && double_data[0] <= bup_end[j]) {
					bup_flag = true;
				}
			}
			if (bup_flag) {
				true_pass_t.push_back(double_data[0]);
				true_pass_x.push_back(double_data[1]);
				true_pass_y.push_back(double_data[2]);
			}

			vector<double> delta_t_list;
			for (int j = 0; j < true_points.size(); j++) {
				vector<string> string_data_ = split(true_points[j], ',');
				vector<double> double_data_(string_data_.size());
				transform(string_data_.begin(), string_data_.end(), double_data_.begin(), [](const string& val) {
					return stod(val);
					});
				double delta_t = abs(double_data[0] - double_data_[0]);
				delta_t_list.push_back(delta_t);
			}
			double delta_t_min = *min_element(delta_t_list.begin(), delta_t_list.end());
			true_pass_d.push_back(delta_t_min);
		}
	}
	true_pass_eval.push_back(true_pass_t);
	true_pass_eval.push_back(true_pass_x);
	true_pass_eval.push_back(true_pass_y);
	true_pass_eval.push_back(true_pass_d);

	vector<double> error_m_s_list;
	double cal_count = 0.0;
	for (int i = 0; i < true_pass_eval[0].size(); i++) {
		int true_timestamp = true_pass_eval[0][i];
		for (int j = 1; j < tra_data.size(); j++) {
			vector<string> string_data = split(tra_data[j], ',');
			vector<double> double_data(string_data.size());
			transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
				return stod(val);
				});
			int tra_timestamp = double_data[0];
			if (true_timestamp == tra_timestamp) {
				double error_m_s = hypot(true_pass_eval[1][i] - double_data[1], true_pass_eval[2][i] - double_data[2]) / true_pass_eval[3][i];
				error_m_s_list.push_back(error_m_s);
				cal_count++;
				break;
			}
		}
	}
	sort(error_m_s_list.begin(), error_m_s_list.end());
	double error_median = error_m_s_list[error_m_s_list.size() / 2];
	double score_cal = -(100.0 / 1.95) * error_median + (200.0 / 1.95);
	score = score_cal * (cal_count / true_pass_eval[0].size());
	if (score > 100.0) {
		score = 100.0;
	}

	score_eCDF.push_back(score);
	for (int i = 0; i < error_m_s_list.size(); i++) {
		score_eCDF.push_back(error_m_s_list[i]);
	}

	return score_eCDF;
}

double E_obstacle(vector<string> tra_data, vector<vector<int>> obstacle, vector<double> map) {
	cout << "Calculat obstacle.." << endl;
	double score = 0.0;
	double x_block_m = obstacle[0].size() / map[0];
	double y_block_m = obstacle.size() / map[1];

	int count = 0;
	int total_count = 0;
	for (int i = 1; i < tra_data.size() - 1; i++) {
		vector<string> string_data = split(tra_data[i], ',');
		vector<double> double_data(string_data.size());
		transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
			return stod(val);
			});
		vector<string> string_data_t1 = split(tra_data[i + 1], ',');
		vector<double> double_data_t1(string_data_t1.size());
		transform(string_data_t1.begin(), string_data_t1.end(), double_data_t1.begin(), [](const string& val) {
			return stod(val);
			});

		int x_block_num = double_data[1] * x_block_m;
		int y_block_num = (map[1] - double_data[2]) * y_block_m;
		int x_block_num_t1 = double_data_t1[1] * x_block_m;
		int y_block_num_t1 = (map[1] - double_data_t1[2]) * y_block_m;

		if (x_block_num - x_block_num_t1 == 0) {
			if (y_block_num < y_block_num_t1) {
				for (int y = y_block_num; y < y_block_num_t1; y++) {
					total_count++;
					bool obstacle_flag = false;
					if ((x_block_num < obstacle[0].size() && x_block_num >= 0) && (y < obstacle.size() && y >= 0)) {
						if (obstacle[y][x_block_num] == 1) {
							obstacle_flag = true;
							for (int j = -3; j <= 3; j++) {
								for (int k = -3; k <= 3; k++) {
									if ((x_block_num + k < obstacle[0].size() && x_block_num + k >= 0) && (y + j < obstacle.size() && y + j >= 0)) {
										if (obstacle[y + j][x_block_num + k] == 0) {
											obstacle_flag = false;
											break;
										}
									}
								}
							}
						}
					}
					else {
						obstacle_flag = true;
					}
					if (!obstacle_flag) {
						count++;
					}
				}
			}
			else {
				for (int y = y_block_num; y > y_block_num_t1; y--) {
					total_count++;
					bool obstacle_flag = false;
					if ((x_block_num < obstacle[0].size() && x_block_num >= 0) && (y < obstacle.size() && y >= 0)) {
						if (obstacle[y][x_block_num] == 1) {
							obstacle_flag = true;
							for (int j = -3; j <= 3; j++) {
								for (int k = -3; k <= 3; k++) {
									if ((x_block_num + k < obstacle[0].size() && x_block_num + k >= 0) && (y + j < obstacle.size() && y + j >= 0)) {
										if (obstacle[y + j][x_block_num + k] == 0) {
											obstacle_flag = false;
											break;
										}
									}
								}
							}
						}
					}
					else {
						obstacle_flag = true;
					}
					if (!obstacle_flag) {
						count++;
					}
				}
			}
		}
		else {
			double a = (y_block_num - y_block_num_t1) / (x_block_num - x_block_num_t1);
			double b = y_block_num - (a * x_block_num);

			if (x_block_num < x_block_num_t1) {
				for (int x = x_block_num; x < x_block_num_t1; x++) {
					total_count++;
					int y = a * x + b;
					bool obstacle_flag = false;
					if ((x < obstacle[0].size() && x >= 0) && (y < obstacle.size() && y >= 0)) {
						if (obstacle[y][x] == 1) {
							obstacle_flag = true;
							for (int j = -3; j <= 3; j++) {
								for (int k = -3; k <= 3; k++) {
									if ((x + k < obstacle[0].size() && x + k >= 0) && (y + j < obstacle.size() && y + j >= 0)) {
										if (obstacle[y + j][x + k] == 0) {
											obstacle_flag = false;
											break;
										}
									}
								}
							}
						}
					}
					else {
						obstacle_flag = true;
					}
					if (!obstacle_flag) {
						count++;
					}
				}
			}
			else {
				for (int x = x_block_num; x > x_block_num_t1; x--) {
					total_count++;
					int y = a * x + b;
					bool obstacle_flag = false;
					if ((x < obstacle[0].size() && x >= 0) && (y < obstacle.size() && y >= 0)) {
						if (obstacle[y][x] == 1) {
							obstacle_flag = true;
							for (int j = -3; j <= 3; j++) {
								for (int k = -3; k <= 3; k++) {
									if ((x + k < obstacle[0].size() && x + k >= 0) && (y + j < obstacle.size() && y + j >= 0)) {
										if (obstacle[y + j][x + k] == 0) {
											obstacle_flag = false;
											break;
										}
									}
								}
							}
						}
					}
					else {
						obstacle_flag = true;
					}
					if (!obstacle_flag) {
						count++;
					}
				}
			}
		}
	}
	score = 100.0 * count / total_count;

	return score;
}

#define PEDESTRIAN_VELOCITY 1.5
double E_velocity(vector<string> tra_data) {
	cout << "Calculat velocity.." << endl;
	double score = 0.0;

	int count = 0;
	for (int i = 1; i < tra_data.size() - 1; i++) {
		vector<string> string_data = split(tra_data[i], ',');
		vector<double> double_data(string_data.size());
		transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
			return stod(val);
			});
		vector<string> string_data_t1 = split(tra_data[i + 1], ',');
		vector<double> double_data_t1(string_data_t1.size());
		transform(string_data_t1.begin(), string_data_t1.end(), double_data_t1.begin(), [](const string& val) {
			return stod(val);
			});

		double velocity = hypot(double_data_t1[1] - double_data[1], double_data_t1[2] - double_data[2]) / (double_data_t1[0] - double_data[0]);
		if (velocity < PEDESTRIAN_VELOCITY) {
			count++;
		}
	}
	score = 100.0 * count / (tra_data.size() - 2);

	return score;
}

double E_frequencies(vector<string> tra_data, vector<vector<string>> sens_start_end) {
	cout << "Calculat frequencies.." << endl;
	double score = 0.0;

	int count = 0;
	for (int i = 1; i < tra_data.size() - 1; i++) {
		vector<string> string_data = split(tra_data[i], ',');
		vector<double> double_data(string_data.size());
		transform(string_data.begin(), string_data.end(), double_data.begin(), [](const string& val) {
			return stod(val);
			});
		vector<string> string_data_t1 = split(tra_data[i + 1], ',');
		vector<double> double_data_t1(string_data_t1.size());
		transform(string_data_t1.begin(), string_data_t1.end(), double_data_t1.begin(), [](const string& val) {
			return stod(val);
			});

		double delta_t = double_data_t1[0] - double_data[0];
		if (delta_t <= 1) {
			count++;
		}
	}

	double start_delta_t = 0.0;
	double end_delta_t = 0.0;
	for (int i = 0; i < sens_start_end.size(); i++) {
		if (tra_data[0] == sens_start_end[i][0]) {
			start_delta_t = stod(tra_data[1]) - stod(sens_start_end[i][1]);
			end_delta_t = stod(sens_start_end[i][2]) - stod(tra_data[tra_data.size() - 1]);
			break;
		}
	}
	if (start_delta_t < 0.0) {
		start_delta_t = 0.0;
	}
	if (end_delta_t < 0.0) {
		end_delta_t = 0.0;
	}
	int count_error = start_delta_t + end_delta_t;
	count = count - count_error;

	score = 100.0 * count / (tra_data.size() - 2);

	return score;
}

double E_staying_area(vector<string> area_data, vector<string> area_division, vector<vector<string>> sens_start_end) {
	cout << "Calculat staying area.." << endl;
	double score = 0.0;

	int total_count = 0;
	double count = 0.0;
	for (int i = 0; i < area_division.size(); i++) {
		vector<string> area_division_string_data = split(area_division[i], ',');

		if (area_division_string_data.size() > 2) {
			total_count++;
			if (i < area_data.size()) {
				vector<string> area_data_string_data = split(area_data[i], ',');

				if (stoi(area_division_string_data[1]) == stoi(area_data_string_data[1])) {
					count = count + 0.75;
				}
				if (area_division_string_data[2] == area_data_string_data[2]) {
					count = count + 0.25;
				}
			}
		}
	}
	score = 100.0 * count / total_count;

	return score;
}

#define N 30
double E_service_operation(vector<string> soe_data, vector<string> soe_distinction) {
	cout << "Calculat service operation.." << endl;
	double score = 0.0;

	int count = 0;
	for (int i = 0; i < soe_distinction.size(); i++) {
		vector<string> soe_distinction_string_data = split(soe_distinction[i], ',');
		for (int j = 1; j < soe_data.size(); j++) {
			vector<string> soe_data_string_data = split(soe_data[j], ',');
			if (stoi(soe_distinction_string_data[0]) + N == stoi(soe_data_string_data[0]) && soe_distinction_string_data[1] == soe_data_string_data[1]) {
				count++;
			}
		}
	}
	score = 100.0 * count / soe_distinction.size();

	return score;
}

//Calculate scare end-------------------------------------------------------------------------------------------------


#define DEFAULT_COUNT 5
int main(int argc, char* argv[]) {
	cout << "Please select a track." << endl;
	cout << "1.Manufacturing" << endl << "2.Restaurant" << endl;
	bool loop = true;
	string selected_track;
	while (loop) {
		selected_track = getchar();
		getchar();

		if (selected_track == "1") {
			selected_track = "Manufacturing";
			loop = false;
		}
		else if (selected_track == "2") {
			selected_track = "Restaurant";
			loop = false;
		}
		else {
			cout << "Please enter 1 or 2." << endl;
		}
	}

	vector<string> Total_socre_list;

	vector<string> conf = Config(selected_track);
	int team_count = conf.size() - DEFAULT_COUNT;
	vector<double> map = Map_size(conf[BASE_DNAME], conf[MAP_SIZE_FNAME]);
	vector<vector<int>> obstacle = Obstacle_data(conf[BASE_DNAME], conf[OBSTACLE_FNAME]);
	vector<vector<string>> sens_start_end = Sens_start_end_data(conf[BASE_DNAME], conf[SENS_START_END_FNAME]);
	vector<vector<string>> tra_data;
	vector<vector<string>> area_data;
	vector<vector<string>> soe_data;
	
	ofstream ofs(".\\" + selected_track + "_total_score.csv");
	ofs << "dir_name,Total_socre,E_m_avg,E_a_avg,E_o_avg,E_v_avg,E_f_avg,E_optional" << endl;
	ofstream ofs_median_accum_error_avg(".\\" + selected_track + "median_accum_error_avg.csv");
	ofs_median_accum_error_avg << "dir_name,median_error_avg[m],accum_error_avg[m/s]" << endl;
	for (int count = 0; count < team_count; count++) {
		vector<string> tra_name_list;
		vector<double> E_m_list;
		vector<double> E_a_list;
		vector<double> E_o_list;
		vector<double> E_v_list;
		vector<double> E_f_list;
		double Total_socre = 0.0;

		string score_dir = ".\\" + conf[DEFAULT_COUNT + count] + "\\score";
		struct stat statBuf;
		if (stat(score_dir.c_str(), &statBuf) == 0 || _mkdir(score_dir.c_str()) == 0) {

			tra_data = Trajectory(conf[DEFAULT_COUNT + count]);

			if (selected_track == "Manufacturing") {
				cout << "-This directory is Manufacturing Track-" << endl;
				ofstream ofs_team(score_dir + "\\score.csv");
				ofs_team << "file_name,E_m,E_a,E_o,E_v,E_f,E_area" << endl;
				//ofstream ofs_team_all_median_eCDF(".\\" + conf[DEFAULT_COUNT + count] + "_all_median_eCDF.txt");
				//ofstream ofs_team_all_accum_eCDF(".\\" + conf[DEFAULT_COUNT + count] + "_all_accum_eCDF.txt");
				area_data = Area(conf[DEFAULT_COUNT + count]);
				vector<double> E_area_list;
				vector<double> all_median_eCDF;
				vector<double> all_accum_eCDF;
				double E_m_avg = 0.0;
				double E_a_avg = 0.0;
				double E_o_avg = 0.0;
				double E_v_avg = 0.0;
				double E_f_avg = 0.0;
				double E_area_avg = 0.0;
				double all_median = 0.0;
				double all_accum = 0.0;
				for (int file_count = 0; file_count < tra_data.size(); file_count++) {
					ofstream ofs_median_error_eCDF(score_dir + "\\median_error_eCDF_" + tra_data[file_count][0]);
					ofs_median_error_eCDF << "error[m]" << endl;
					ofstream ofs_accum_error_eCDF(score_dir + "\\accum_error_eCDF_" + tra_data[file_count][0]);
					ofs_accum_error_eCDF << "error[m/s]" << endl;
					vector<string> true_points = True_points_data(conf[BASE_DNAME], tra_data[file_count][0]);
					vector<string> true_pass = True_pass_data(conf[BASE_DNAME], tra_data[file_count][0]);
					vector<string> bup_info = BUP_info_data(conf[BASE_DNAME], tra_data[file_count][0]);

					cout << tra_data[file_count][0] << endl;
					tra_name_list.push_back(tra_data[file_count][0]);
					vector<double> E_m = E_median_error(tra_data[file_count], true_points, true_pass, bup_info);
					E_m_list.push_back(E_m[0]);
					for (int i = 1; i < E_m.size(); i++) {
						ofs_median_error_eCDF << E_m[i] << endl;
						all_median_eCDF.push_back(E_m[i]);
					}
					vector<double> E_a = E_accum_error(tra_data[file_count], true_points, true_pass, bup_info);
					E_a_list.push_back(E_a[0]);
					for (int i = 1; i < E_a.size(); i++) {
						ofs_accum_error_eCDF << E_a[i] << endl;
						all_accum_eCDF.push_back(E_a[i]);
					}
					double E_o = E_obstacle(tra_data[file_count], obstacle, map);
					E_o_list.push_back(E_o);
					double E_v = E_velocity(tra_data[file_count]);
					E_v_list.push_back(E_v);
					double E_f = E_frequencies(tra_data[file_count], sens_start_end);
					E_f_list.push_back(E_f);

					vector<string> area_division = Area_division_data(conf[OPTIONAL_DNAME], area_data[file_count][0]);
					double E_area = E_staying_area(area_data[file_count], area_division, sens_start_end);
					E_area_list.push_back(E_area);

					ofs_team << tra_data[file_count][0] + "," + to_string(E_m[0]) + "," + to_string(E_a[0]) + "," + to_string(E_o) + "," + to_string(E_v) + "," + to_string(E_f) + "," + to_string(E_area) << endl;

					E_m_avg = E_m_avg + E_m[0];
					E_a_avg = E_a_avg + E_a[0];
					E_o_avg = E_o_avg + E_o;
					E_v_avg = E_v_avg + E_v;
					E_f_avg = E_f_avg + E_f;
					E_area_avg = E_area_avg + E_area;

					double median = E_m[((E_m.size() - 1) / 2) + 1];
					double accum = E_a[((E_a.size() - 1) / 2) + 1];
					all_median = all_median + median;
					all_accum = all_accum + accum;
				}
				E_m_avg = E_m_avg / E_m_list.size();
				E_a_avg = E_a_avg / E_a_list.size();
				E_o_avg = E_o_avg / E_o_list.size();
				E_v_avg = E_v_avg / E_v_list.size();
				E_f_avg = E_f_avg / E_f_list.size();
				E_area_avg = E_area_avg / E_area_list.size();

				sort(all_median_eCDF.begin(), all_median_eCDF.end());
				for (int i = 0; i < all_median_eCDF.size(); i++) {
					//ofs_team_all_median_eCDF << all_median_eCDF[i] << endl;
				}
				sort(all_accum_eCDF.begin(), all_accum_eCDF.end());
				for (int i = 0; i < all_accum_eCDF.size(); i++) {
					//ofs_team_all_accum_eCDF << all_accum_eCDF[i] << endl;
				}

				double median_avg = all_median / tra_data.size();
				double accum_avg = all_accum / tra_data.size();
				ofs_median_accum_error_avg << conf[DEFAULT_COUNT + count] + "," + to_string(median_avg) + "," + to_string(accum_avg) << endl;

				Total_socre = 0.25 * E_m_avg + 0.25 * E_a_avg + 0.15 * E_o_avg + 0.1 * E_v_avg + 0.1 * E_f_avg + 0.15 * E_area_avg;

				Total_socre_list.push_back(conf[DEFAULT_COUNT + count] + "," + to_string(Total_socre) + "," + to_string(E_m_avg) + "," + to_string(E_a_avg) + "," + to_string(E_o_avg) + "," + to_string(E_v_avg) + "," + to_string(E_f_avg) + "," + to_string(E_area_avg));
				ofs << conf[DEFAULT_COUNT + count] + "," + to_string(Total_socre) + "," + to_string(E_m_avg) + "," + to_string(E_a_avg) + "," + to_string(E_o_avg) + "," + to_string(E_v_avg) + "," + to_string(E_f_avg) + "," + to_string(E_area_avg) << endl;
			}
			else {
				cout << "-This directory is Restaurant Track-" << endl;
				ofstream ofs_team(score_dir + "\\score.csv");
				ofs_team << "file_name,E_m,E_a,E_o,E_v,E_f" << endl;
				//ofstream ofs_team_all_median_eCDF(".\\" + conf[DEFAULT_COUNT + count] + "_all_median_eCDF.txt");
				//ofstream ofs_team_all_accum_eCDF(".\\" + conf[DEFAULT_COUNT + count] + "_all_accum_eCDF.txt");
				soe_data = Soe(conf[DEFAULT_COUNT + count]);
				double E_soe = 0.0;
				vector<double> all_median_eCDF;
				vector<double> all_accum_eCDF;
				double E_m_avg = 0.0;
				double E_a_avg = 0.0;
				double E_o_avg = 0.0;
				double E_v_avg = 0.0;
				double E_f_avg = 0.0;
				double all_median = 0.0;
				double all_accum = 0.0;
				for (int file_count = 0; file_count < tra_data.size(); file_count++) {
					ofstream ofs_median_error_eCDF(score_dir + "\\median_error_eCDF_" + tra_data[file_count][0]);
					ofs_median_error_eCDF << "error[m]" << endl;
					ofstream ofs_accum_error_eCDF(score_dir + "\\accum_error_eCDF_" + tra_data[file_count][0]);
					ofs_accum_error_eCDF << "error[m/s]" << endl;
					vector<string> true_points = True_points_data(conf[BASE_DNAME], tra_data[file_count][0]);
					vector<string> true_pass = True_pass_data(conf[BASE_DNAME], tra_data[file_count][0]);
					vector<string> bup_info = BUP_info_data(conf[BASE_DNAME], tra_data[file_count][0]);

					cout << tra_data[file_count][0] << endl;
					tra_name_list.push_back(tra_data[file_count][0]);
					vector<double> E_m = E_median_error(tra_data[file_count], true_points, true_pass, bup_info);
					E_m_list.push_back(E_m[0]);
					for (int i = 1; i < E_m.size(); i++) {
						ofs_median_error_eCDF << E_m[i] << endl;
						all_median_eCDF.push_back(E_m[i]);
					}
					vector<double> E_a = E_accum_error(tra_data[file_count], true_points, true_pass, bup_info);
					E_a_list.push_back(E_a[0]);
					for (int i = 1; i < E_a.size(); i++) {
						ofs_accum_error_eCDF << E_a[i] << endl;
						all_accum_eCDF.push_back(E_a[i]);
					}
					double E_o = E_obstacle(tra_data[file_count], obstacle, map);
					E_o_list.push_back(E_o);
					double E_v = E_velocity(tra_data[file_count]);
					E_v_list.push_back(E_v);
					double E_f = E_frequencies(tra_data[file_count], sens_start_end);
					E_f_list.push_back(E_f);

					ofs_team << tra_data[file_count][0] + "," + to_string(E_m[0]) + "," + to_string(E_a[0]) + "," + to_string(E_o) + "," + to_string(E_v) + "," + to_string(E_f) << endl;

					E_m_avg = E_m_avg + E_m[0];
					E_a_avg = E_a_avg + E_a[0];
					E_o_avg = E_o_avg + E_o;
					E_v_avg = E_v_avg + E_v;
					E_f_avg = E_f_avg + E_f;

					double median = E_m[((E_m.size() - 1) / 2) + 1];
					double accum = E_a[((E_a.size() - 1) / 2) + 1];
					all_median = all_median + median;
					all_accum = all_accum + accum;
				}
				for (int file_count = 0; file_count < soe_data.size(); file_count++) {
					vector<string> soe_distinction = Soe_distinction_data(conf[OPTIONAL_DNAME], soe_data[file_count][0]);
					E_soe = E_service_operation(soe_data[file_count], soe_distinction);

					ofs_team << endl;
					ofs_team << soe_data[file_count][0] + "," + to_string(E_soe) << endl;
				}
				E_m_avg = E_m_avg / E_m_list.size();
				E_a_avg = E_a_avg / E_a_list.size();
				E_o_avg = E_o_avg / E_o_list.size();
				E_v_avg = E_v_avg / E_v_list.size();
				E_f_avg = E_f_avg / E_f_list.size();

				sort(all_median_eCDF.begin(), all_median_eCDF.end());
				for (int i = 0; i < all_median_eCDF.size(); i++) {
					//ofs_team_all_median_eCDF << all_median_eCDF[i] << endl;
				}
				sort(all_accum_eCDF.begin(), all_accum_eCDF.end());
				for (int i = 0; i < all_accum_eCDF.size(); i++) {
					//ofs_team_all_accum_eCDF << all_accum_eCDF[i] << endl;
				}

				double median_avg = all_median / tra_data.size();
				double accum_avg = all_accum / tra_data.size();
				ofs_median_accum_error_avg << conf[DEFAULT_COUNT + count] + "," + to_string(median_avg) + "," + to_string(accum_avg) << endl;

				Total_socre = 0.25 * E_m_avg + 0.2 * E_a_avg + 0.15 * E_o_avg + 0.1 * E_v_avg + 0.1 * E_f_avg + 0.2 * E_soe;

				Total_socre_list.push_back(conf[DEFAULT_COUNT + count] + "," + to_string(Total_socre) + "," + to_string(E_m_avg) + "," + to_string(E_a_avg) + "," + to_string(E_o_avg) + "," + to_string(E_v_avg) + "," + to_string(E_f_avg) + "," + to_string(E_soe));
				ofs << conf[DEFAULT_COUNT + count] + "," + to_string(Total_socre) + "," + to_string(E_m_avg) + "," + to_string(E_a_avg) + "," + to_string(E_o_avg) + "," + to_string(E_v_avg) + "," + to_string(E_f_avg) + "," + to_string(E_soe) << endl;
			}
		}else {
			cerr << "Creating a folder failed." << endl;
		}
	}
	cout << "Finish!" << endl;
	cout << endl << "Please press enter key.." << endl;
	getchar();
}