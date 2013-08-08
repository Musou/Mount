// Mount.cpp : Defines the entry point for the console application.
//

#include "stdafx.h"
#include <iostream>
#include <fstream>
#include <algorithm>
#include <numeric>
#include <string>
#include <vector>
#include <filesystem>
#include <boost/filesystem.hpp>
#include <boost/foreach.hpp>
#include <boost/cstdint.hpp>
#include <boost/bind.hpp>

#define MAX_FILES 10000
namespace bfs = boost::filesystem;
using namespace std;
using namespace std::tr2::sys;
long long psize;

struct file {
	std::string name;
	long long size;
};

bool sizesort(file const& lhs, file const&rhs) {
		return lhs.size>rhs.size;
}

void  getFoldersize(std::string rootFolder,long long & f_size, int &numfiles) {
   path folderPath(rootFolder);                      
   if (exists(folderPath))	{
        directory_iterator end_itr;
        for (directory_iterator dirIte(rootFolder); dirIte != end_itr; ++dirIte )	{
			path filePath(complete (dirIte->path(), folderPath));
			try{
				if (!is_directory(dirIte->status()) ) {
					if(file_size(filePath)<psize){
						f_size = f_size + file_size(filePath);
						++numfiles;
					}
				} else {
					  getFoldersize(filePath,f_size,numfiles);
				}
			}catch(exception& e){  cout << e.what() << endl; }
		}
	}
}

void getFileNames(bfs::path fpath, std::vector<file> &sizes) {
	if(bfs::is_directory(fpath)) {
		bfs::directory_iterator it(fpath),eod;
		BOOST_FOREACH(bfs::path const &p, std::make_pair(it,eod))
		{
			if(is_regular_file(p)) {
				file temp;
				temp.size=bfs::file_size(p);
				temp.name=p.string();
				if(temp.size<psize) {
					sizes.push_back(temp);
				}
			} else if(is_directory(p)) {
				getFileNames(p,sizes);
			}
		}
	}
}


int _tmain(int argc, _TCHAR* argv[])
{
	ifstream infile;
	ofstream outfile;
	infile.open("Mount.ini");
	outfile.open("files.txt");
	infile >> psize;
	long long maxsize=psize*32;
	std::vector<file> sizes;
	string initpath;
	getline(infile,initpath);
	getline(infile,initpath);
	int numdirs;
	infile >> numdirs;
	vector<string> dirs;
	string tmpdr;
	getline(infile,tmpdr);
	for(int i=0;i<numdirs;++i) {
		getline(infile,tmpdr);
		dirs.push_back(tmpdr);
	}
	bfs::path fpath (bfs::initial_path());
	fpath = bfs::system_complete(bfs::path(initpath,bfs::native));
	int numfiles=0;
	long long f_size=0;
	getFoldersize(initpath,f_size,numfiles);

	if(f_size>maxsize) {
		cout << "Files too large" << endl;
		return 0;
	}

	getFileNames(fpath,sizes);

	std::sort(sizes.begin(),sizes.end(),&sizesort);
	long long csum=0;
	int nf=0;
	int ptn=1;
	while(!sizes.empty()) {
		if(ptn>1)
			outfile << endl;
		outfile << "[Drive " << ptn << "]";
		for(std::vector<file>::iterator it=sizes.begin();it!=sizes.end();) {
			if(csum+it->size<psize){
				csum+=it->size;
				outfile << endl << it->name;
				it=sizes.erase(it);
				++nf;
			} else {
				++it;
			}
		}
		csum=0;
		ptn++;
		}
	if(nf!=numfiles) {
		cout << "Error: unable to distribute all files";
		return 1;
	}

	infile.close();
	outfile.close();
	ifstream files;
	files.open("files.txt");
	string fname;
	int drnum=0;
	while(!files.eof()) {
		getline(files,fname);
		if(*fname.begin()=='[' && *fname.rbegin()==']') {
			drnum=atoi(fname.substr(7,fname.length()-8).c_str())-1;
		} else {
			string command="copy \"";
			//command.append(initpath);
			command.append(fname);
			command.append("\"");
			command.append(" \"");
			command.append(dirs.at(drnum));
			command.append("\"");
			system(command.c_str());
		}
	}
	nf=0;
	long long f_size2;
	for(int i=0;i<numdirs;++i) {
		getFoldersize(dirs.at(i),f_size2,nf);
	}
	if(nf==numfiles) {
		cout << "All files copied successfully." << endl;
	} else {
		cout << numfiles-nf << " files were not able to be copied." << endl;
	}
	system("pause");
	return 0;
}

