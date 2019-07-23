#include<iostream>
#include<vector>
#include<math.h>
#include<map>
using namespace std;
#define delT 5
#define TINF 9e8
struct SubRecord//deltaT�Լ�������Hi�Ǽ���õ� 
{
	long long time;
	string src_ip;
	string dst_ip;
	int src_port;
	int dst_port;
	int packet_total;
	int packet_suc;
	int byte_sum;
	float CPUi;
	string Si;
};
vector<SubRecord> Record,ResultR;
map<string,int> IPSrcMap;//ͳ��ÿһ��ip��ַ���ֵĸ�����������ֵʱʹ��
map<int,int>PortSrcMap;//ͳ��ÿһ���˿ڳ��ֵĸ�����������ֵʱʹ��
map<string,int>::iterator Iter;
map<int,int>::iterator IterInt;

long long StrToInt(string s);//���ַ���������תΪ���Ͳ��������ͽ��
string GetString(FILE *f);//���ļ��ж���һ���ַ����������ض������ַ��� 
void Process1Logic(FILE *f);
void Process2Logic(FILE *f,FILE *resf);
void ProcessPortInfo(FILE *f);
int main()
{
	FILE *fp1,*fp2,*resfp;//fp1��Ӧ�ڴ洢�˿��յ�ת�����ݰ�����Ϣ��fp2��Ӧ���ݰ�Դ�˿ڡ�Ŀ�Ķ˿ڵ���Ϣ�����´����б�ʶ��1���ͱ�ʶ��2������Ķ���Ҳ��� 
	if((fp1=fopen("openflow.txt","r"))==NULL)//packet_in�Ǵ���֮ǰ���ļ��� 
	{
		printf("The file can not be open!");
    	exit(1);//���������ִ��
	} 
	if((fp2=fopen("mydebug","r"))==NULL)//packet_in�Ǵ���֮ǰ���ļ��� //TODO �ļ��� 
	{
		printf("The file can not be open!");
    	exit(1);//���������ִ��
	} 
	if((resfp=fopen("ovs_result/ovs_vector.txt","w"))==NULL)//packet_in�Ǵ���֮ǰ���ļ��� 
	{
		printf("The file can not be open!");
    	exit(1);//���������ִ��
	} 
	while(!feof(fp1)){Process1Logic(fp1);}//���ļ� 
	while(!feof(fp2)){Process2Logic(fp2,resfp);}//���ļ� 
	return 0;
} 
void Process2Logic(FILE *f,FILE *resf)
{
	int i=0,begintime,j,k=0,PktNum=0;
	double temp1,temp2,PortSrcEntropy=0,IPSrcEntropy=0;
	bool flag; 
	string temps;
	SubRecord SR;
	fprintf(resf,"Timetag,src_ip,dst_ip,src_port,deltaT,packet_total,packet_suc,byte_sum,Hi,CPUi,Si\n");
	while(!feof(f))
	{
		temps=GetString(f); SR.time=StrToInt(temps);
		temps=GetString(f); SR.src_ip=temps;
		temps=GetString(f); SR.dst_ip=temps;
		temps=GetString(f); SR.src_port=(int)StrToInt(temps);
		temps=GetString(f); SR.dst_port=(int)StrToInt(temps);
		fscanf(f,"%f %f",&temp1,&temp2);
		SR.CPUi=temp1+temp2;
		fscanf(f,"%f %f\n",&temp1,&temp2);
		//temps=GetString(f); SR.CPUi=(float)StrToInt(temps);
		Record.push_back(SR);
	}
	SR.time=TINF;
	Record.push_back(SR);
   	//for(i=0;i<Record.size();i++) cout<<i<<" "<<Record[i].time<<endl;
	for(i=0,begintime=Record[0].time;i<Record.size();i++)
	{
		if(Record[i].time-begintime<delT)
		{
			PktNum++;
			for(j=0,flag=false;j<ResultR.size();j++)
			{
				if((ResultR[j].src_ip==Record[i].src_ip)&&(ResultR[j].dst_ip==Record[i].dst_ip))
				{
					ResultR[j].packet_total++;flag=true;
					break;
				}
			}
			if(!flag){SR=Record[i];SR.packet_total=1;ResultR.push_back(SR);}
			
			if(IPSrcMap.find(Record[i].src_ip)==IPSrcMap.end()) {
				IPSrcMap.insert(pair<string,int>(Record[i].src_ip,1));
			} else IPSrcMap[Record[i].src_ip]++;
			if(PortSrcMap.find(Record[i].src_port)==PortSrcMap.end()) {
				PortSrcMap.insert(pair<int,int>(Record[i].src_port,1));
			} else PortSrcMap[Record[i].src_port]++;
		}
		else//һ��ʱ�������� //��ֵ���㣬�����ĸΪ0���򲻽��г��������ֱ��Ϊ0 
		{
			cout<<PktNum<<" ";
			begintime+=delT;
			if(i!=Record.size()-1)i--;
			for(Iter=IPSrcMap.begin(); Iter!=IPSrcMap.end(); Iter++)IPSrcEntropy+=((double)Iter->second/PktNum)*log((double)Iter->second/PktNum);
			for(IterInt=PortSrcMap.begin(); IterInt!=PortSrcMap.end(); IterInt++) PortSrcEntropy+=((double)IterInt->second/PktNum)*log((double)IterInt->second/PktNum);
			//cout<<IPSrcEntropy<<" "<<PortSrcEntropy<<endl;
			for(int j=0;j<ResultR.size();j++)
			{
				if(PortSrcEntropy!=0)fprintf(resf,"%d-%d,%s,%s,%d,%d,%d,,,%.5f,%f,\n",begintime-delT,begintime,ResultR[j].src_ip.c_str(),ResultR[j].dst_ip.c_str(),ResultR[j].src_port,delT,ResultR[j].packet_total,IPSrcEntropy/PortSrcEntropy,ResultR[j].CPUi);
				else fprintf(resf,"%d-%d,%s,%s,%d,%d,%d,,,0.00000,%f,\n",begintime-delT,begintime,ResultR[j].src_ip.c_str(),ResultR[j].dst_ip.c_str(),ResultR[j].src_port,delT,ResultR[j].packet_total,ResultR[j].CPUi);//cout<<"time="<<ResultR[j].time<<"  "<<ResultR[j].src_ip<<"  "<<ResultR[j].dst_ip<<"  "<<ResultR[j].packet_total<<"  "<<ResultR[j].CPUi<<endl;
			}
			PktNum=0;
			PortSrcMap.clear();
			IPSrcMap.clear();
			IPSrcEntropy=0;
			PortSrcEntropy=0;
			ResultR.clear(); 
		}
	}
}
void Process1Logic(FILE *f)
{
	string TagS;
	while(!feof(f))
	{
		TagS=GetString(f);
		if(TagS=="prots_information")
		{
		//	TagS=ProcessPortInfo(f);
		}
		else if(TagS=="table_information")
		{
			//Todo//ProcessTableInfo(f);
		}
		else if(TagS=="flow_entries_information")
		{
			//Todo//ProcessFlowInfo(f);
		}
	}
}
string GetString(FILE *f)
{
	char c;string temps="";
	while((c=fgetc(f))!=EOF&&c!=' '&&c!='\n')
	{
		temps+=c;
	}
	return temps;
}
long long StrToInt(string s) {
	long long result=0;
	for(int i=0; i<s.size(); i++) {
		result+=(s[i]-'0')*pow(10,s.size()-1-i);
	}
	return result;
}
