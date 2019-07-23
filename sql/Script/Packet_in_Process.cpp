//�ó���ÿ�δ����ļ��������ݿ���д����ʱ�������ԭ��������ɾ����Ҫ��ʵ��׷�ӹ��ܣ���Ҫ�����ݱ������indexɾ����Ȼ����ʱ���Ticktime��Ϊ������ 

#include<iostream>
#include<vector>
#include<map>
#include<math.h>
#include<io.h>
#include<algorithm>
using namespace std;
#define TINF 9e13
#define delT 5
#define filePath  "D:\\Mysql\\sql�ű��ļ�\\packet_in_src"
#define BEGIN 35
struct SubRecord {
	long long time;
	string src_ip;
	string dst_ip;
	int src_port;
	int dst_port;
	int size;
};
vector<string> files;
vector<SubRecord> Record,PartRecord;
map<string,int> IPSrcMap,IPDstMap;//ͳ��ÿһ��ip��ַ���ֵĸ�����������ֵʱʹ��
map<int,int>PortSrcMap,PortDstMap;//ͳ��ÿһ���˿ڳ��ֵĸ�����������ֵʱʹ��
map<string,int>::iterator Iter;
map<int,int>::iterator IterInt;
bool cmp(SubRecord a,SubRecord b){return a.time<b.time;}
int StrToInt(string s);//���ַ���������תΪ���Ͳ��������ͽ��
int Process1(string str);//����ʱ������ڵĵ�һ�������������ꡢ�¡���
int Process2(string str);//����ʱ������ڵĵڶ�������������ʱ���֡���
string GetString(FILE *f);//���ļ��ж���һ���ַ����������ض������ַ���
void ProcessOneLine(FILE *f,FILE *tempf);//�����ļ��е�һ�м�¼���õ�һ���ṹ�壨SubRecord�������ѽṹ��ѹ������Record�У�ͬʱ���ýṹ���ÿһ����Աд���м������ļ�
void GainRecord(FILE *f,vector<SubRecord> Record);//��������Record��������ֵ���õ�����ÿ����¼�ĸ������ݣ��������ݴ���fָ����ļ���//ע�⣺Ҫ��Record�еļ�¼�ǰ���time�������� 
void getFiles( string path, vector<string>& files );
void CreateTable(string temps,FILE *SQLfp);
int main() {
	int i=0,j=0;
	SubRecord Tail;
	Tail.time=TINF;
	FILE *fp,*resfp,*tempfp,*SQLfp;

	getFiles(filePath, files);

	if((resfp=fopen("packet_in_result\\packet_in_vector.txt","w"))==NULL) {
		printf("The file can not be open!");
		exit(1);//���������ִ��
	}
	if((tempfp=fopen("packet_in_result\\packet_in_info.txt","w"))==NULL) {
		printf("The file can not be open!");
		exit(1);//���������ִ��
	}
	if((SQLfp=fopen("Mysql.sql","w"))==NULL) {
		printf("The file can not be open!");
		exit(1);//���������ִ��
	}
	fputs("time,src_ip,dst_ip,src_port,dst_port,size\n",tempfp);//�м������ļ�

	for(int i=0; i<files.size(); i++) {
		PartRecord.clear();
		if((fp=fopen(files[i].c_str(),"r"))==NULL) {
			printf("The file can not be open!");
			exit(1);//���������ִ��
		}
		while(!feof(fp)) {
			ProcessOneLine(fp,tempfp);
		}
		PartRecord.push_back(Tail);
		FILE *tempf;
		string temps="packet_in_result\\";
		for(j=BEGIN; j<files[i].size(); j++)temps+=files[i][j];
		tempf=fopen(temps.c_str(),"w");
		GainRecord(tempf,PartRecord);
		CreateTable(temps,SQLfp);
	}
//	if((fp=fopen("result.txt","r"))==NULL)//packet_in�Ǵ���֮ǰ���ļ���
//	{
//		printf("The file can not be open!");
//    	exit(1);//���������ִ��
//	}
//	while(!feof(fp))
//	{
//		j++;
//		ProcessOneLine(fp,tempfp,j);
//	}
	Record.push_back(Tail);//ѹ��һ��β���ݣ��ڵ㣩��ʱ��ȡֵ����Ϊ�������
	sort(Record.begin(),Record.end(),cmp);
	GainRecord(resfp,Record);
	//cout<<Record.size();
	CreateTable("packet_in_result\\packet_in_vector.txt",SQLfp);

	cout<<"over�������ļ���packet_in_vector.txt�����ݿ�";
	return 0;
}
void CreateTable(string temps,FILE *SQLfp) {
	int i;
	string Str;
	for(i=17; i<temps.size(); i++)
		if(temps[i]!='.')Str+=temps[i];
		else break;
	fprintf(SQLfp,"DROP TABLE IF EXISTS `%s`;CREATE TABLE `%s`  (`index` int(255) NOT NULL AUTO_INCREMENT,`TickName` varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,",Str.c_str(),Str.c_str());
	fprintf(SQLfp,"`PktNum` int(20) NOT NULL,`PktNumRate` double(255, 2) NOT NULL,`AvgLength` double(255, 2) NOT NULL,");
	fprintf(SQLfp,"`IpEntropy` double(255, 5) NOT NULL,`PortEntropy` varchar(255) CHARACTER SET utf8 COLLATE utf8_general_ci NOT NULL,PRIMARY KEY (`index`) USING BTREE) ENGINE = InnoDB CHARACTER SET = utf8 COLLATE = utf8_general_ci\n");
	fprintf(SQLfp,"ROW_FORMAT = Dynamic;SET FOREIGN_KEY_CHECKS = 1;\n");
	fprintf(SQLfp,"load data infile \"D:\\\\Mysql\\\\sql�ű��ļ�\\\\packet_in_result\\\\%s.txt\" into table %s fields terminated by ',' lines terminated by '\\n' ;",Str.c_str(),Str.c_str());
//	temps="D:\\Mysql\\sql�ű��ļ�\\"+temps;
//	cout<<temps<<endl;
//	fprintf(SQLfp,"load data infile \"%s\" into table %s fields terminated by ',' lines terminated by '\\n' ;",temps.c_str(),Str.c_str());
}
string GetString(FILE *f) {
	char c;
	string temps="";
	while((c=fgetc(f))!=EOF&&c!=' '&&c!='\n') {
		temps+=c;
	}
	return temps;
}
int StrToInt(string s) {
	int result=0;
	for(int i=0; i<s.size(); i++) {
		result+=(s[i]-'0')*pow(10,s.size()-1-i);
	}
	return result;
}
int Process1(string str) {
	int cur=0,result=0;
	string s[3];
	for(int i=0; i<str.size(); i++) {
		if(str[i]!='.') s[cur]+=str[i];
		else cur++;
	}
//	cout<<s[0]<<endl;
	result=StrToInt(s[0])*10000;
	result+=StrToInt(s[1])*100;
	result+=StrToInt(s[2]);
	return result;
}
int Process2(string str) {
	int cur=0,result=0;
	string s[3];
	for(int i=0; i<str.size(); i++) {
		if(str[i]!=':') s[cur]+=str[i];
		else cur++;
	}
	result=StrToInt(s[0])*10000;
	result+=StrToInt(s[1])*100;
	result+=StrToInt(s[2]);
	return result;
}
void ProcessOneLine(FILE *f,FILE *tempf) {
	int i;
	string Str;
	SubRecord SR;
	GetString(f);
	Str=GetString(f);//cout<<Str<<"  ";
	SR.time=(long long)Process1(Str)*1000000;
	Str=GetString(f);//cout<<Str<<endl;
	SR.time+=(long long)Process2(Str);
	//Str=GetString(f);
	//if(Str=="PM"||Str=="pm"||Str=="pM"||Str=="Pm") SR.time+=120000;
	GetString(f);
	SR.src_ip=GetString(f);//cout<<Str<<endl;exit(1);
	GetString(f);
	SR.dst_ip=GetString(f);
	for(i=0; i<3; i++)GetString(f);
	SR.src_port=StrToInt(GetString(f));
	GetString(f);
	SR.dst_port=StrToInt(GetString(f));
	GetString(f);
	SR.size=StrToInt(GetString(f));
	Record.push_back(SR);
	PartRecord.push_back(SR);
	fprintf(tempf,"%lld,%s,%s,%d,%d,%d\n",SR.time,SR.src_ip.c_str(),SR.dst_ip.c_str(),SR.src_port,SR.dst_port,SR.size);
}
void GainRecord(FILE *f,vector<SubRecord> Record) {
	//fputs("TickName,PktNum,PktNumRate,AvgLength,IpEntropy,PortEntropy\n",f);//���������ļ�
	PortDstMap.clear();
	PortSrcMap.clear();
	IPDstMap.clear();
	IPSrcMap.clear();
	int i,PktNum=0,PrePktNum=0,index=0;
	double PktNumRate=0,Avglength=0,IPSrcEntropy=0,IPDstEntropy=0,PortSrcEntropy=0,PortDstEntropy=0;
	long long begintime;//cout<<Record.size()<<" ";
	for(i=0,begintime=Record[0].time,PktNum=0; i<Record.size(); i++) {
		//	cout<<Record[i].time-begintime<<endl;
		if(Record[i].time-begintime<delT) {
			PktNum++;
			Avglength+=Record[i].size;
			if(IPSrcMap.find(Record[i].src_ip)==IPSrcMap.end()) {
				IPSrcMap.insert(pair<string,int>(Record[i].src_ip,1));
			} else IPSrcMap[Record[i].src_ip]++;
			if(IPDstMap.find(Record[i].dst_ip)==IPDstMap.end()) {
				IPDstMap.insert(pair<string,int>(Record[i].dst_ip,1));
			} else IPDstMap[Record[i].dst_ip]++;
			if(PortSrcMap.find(Record[i].src_port)==PortSrcMap.end()) {
				PortSrcMap.insert(pair<int,int>(Record[i].src_port,1));
			} else PortSrcMap[Record[i].src_port]++;
			if(PortDstMap.find(Record[i].dst_port)==PortDstMap.end()) {
				PortDstMap.insert(pair<int,int>(Record[i].dst_port,1));
			} else PortDstMap[Record[i].dst_port]++;
		} else {
			if(PktNum>1) 
			{
				Avglength=Avglength/PktNum;
				if(PrePktNum!=0)PktNumRate=(double)(PrePktNum-PktNum)/PrePktNum;
				else PktNumRate=0;
				for(Iter=IPSrcMap.begin(); Iter!=IPSrcMap.end(); Iter++)	IPSrcEntropy+=((double)Iter->second/PktNum)*log((double)Iter->second/PktNum);
				for(Iter=IPDstMap.begin(); Iter!=IPDstMap.end(); Iter++)	IPDstEntropy+=((double)Iter->second/PktNum)*log((double)Iter->second/PktNum);
				for(IterInt=PortSrcMap.begin(); IterInt!=PortSrcMap.end(); IterInt++)	PortSrcEntropy+=((double)IterInt->second/PktNum)*log((double)IterInt->second/PktNum);
				for(IterInt=PortDstMap.begin(); IterInt!=PortDstMap.end(); IterInt++)	PortDstEntropy+=((double)IterInt->second/PktNum)*log((double)IterInt->second/PktNum);
				fprintf(f,"%d,%lld-%lld,%d,%.2f,%.2f,",++index,begintime,begintime+delT,PktNum,PktNumRate,Avglength);
				if(IPDstEntropy!=0)fprintf(f,"%.5f,",IPSrcEntropy/IPDstEntropy);
				else fprintf(f,"%.5f,",0);
				if(PortDstEntropy!=0)fprintf(f,"%.5f\n",PortSrcEntropy/PortDstEntropy);
				else fprintf(f,"%.5f\n",0);
				if(i!=Record.size()-1)i--;
				PrePktNum=PktNum;
			}
			begintime+=delT;
			PktNum=0;
			Avglength=0;
			PortDstMap.clear();
			PortSrcMap.clear();
			IPDstMap.clear();
			IPSrcMap.clear();
			IPSrcEntropy=0;
			IPDstEntropy=0;
			PortSrcEntropy=0;
			PortDstEntropy=0;
		}
	}
}
void getFiles( string path, vector<string>& files ) {
	//�ļ����
	long   hFile   =   0;
	//�ļ���Ϣ
	struct _finddata_t fileinfo;
	string p;
	if((hFile = _findfirst(p.assign(path).append("\\*").c_str(),&fileinfo)) !=  -1) {
		do {
			//�����Ŀ¼,����֮
			//�������,�����б�
			if((fileinfo.attrib &  _A_SUBDIR)) {
				if(strcmp(fileinfo.name,".") != 0  &&  strcmp(fileinfo.name,"..") != 0)
					getFiles( p.assign(path).append("\\").append(fileinfo.name), files );
			} else {
				files.push_back(p.assign(path).append("\\").append(fileinfo.name) );
			}
		} while(_findnext(hFile, &fileinfo)  == 0);
		_findclose(hFile);
	}
}
