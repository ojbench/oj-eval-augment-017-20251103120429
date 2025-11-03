#include <iostream>
#include <cstdio>

#include <fstream>

#include <string>
#include "util.h"
#include "user.h"
#include "train.h"
#include "order.h"
using namespace std;

static string getArg(const string &line, char key){
    size_t i=0,n=line.size();
    while(i<n){
        while(i<n && line[i]!='-') ++i;
        if(i>=n) break; if(i+1>=n) break; char k=line[i+1];
        size_t j=i+2; while(j<n && line[j]==' ') ++j; size_t s=j;
        while(j<n && line[j]!=' ') ++j; string val=line.substr(s, j-s);
        if(k==key) return val; i=j;
    }
    return string();
}

static void print_time(const Train &t,int baseStartDay,int stationIdx,bool arrive){
    if(arrive){ if(stationIdx==1){ cout<<"xx-xx xx:xx"; return;} int v=t.arr[stationIdx]; int d=baseStartDay+v/1440; int m=v%1440; cout<<formatMMDD_HHMM(d,m); }
    else{ if(stationIdx==t.stationNum){ cout<<"xx-xx xx:xx"; return;} int v=t.dep[stationIdx]; int d=baseStartDay+v/1440; int m=v%1440; cout<<formatMMDD_HHMM(d,m); }
}

static int sum_prices(const Train &t,int l,int r){ int s=0; for(int i=l;i<=r;++i) s+=t.prices[i]; return s; }
static void enqueue_pending(Train &t,int baseStartDay,int oid){
    int di=baseStartDay - t.saleStart; if(di<0||di>=t.dayCount) return; Order &o=orders_get_mut(oid); o.nextPending=-1; if(t.pendHead[di]==-1){ t.pendHead[di]=t.pendTail[di]=oid; } else { orders_get_mut(t.pendTail[di]).nextPending=oid; t.pendTail[di]=oid; }
}
static void process_pending_for_day(Train &t,int baseStartDay){
    int di=baseStartDay - t.saleStart; if(di<0||di>=t.dayCount) return; int cur=t.pendHead[di], prev=-1; while(cur!=-1){ Order &o=orders_get_mut(cur); int can=seats_min(t,baseStartDay,o.fromIdx,o.toIdx-1); int nxt=o.nextPending; if(o.status==0 && can>=o.num){ seats_add(t,baseStartDay,o.fromIdx,o.toIdx-1,-o.num); o.status=1; // remove from queue
            if(prev==-1) t.pendHead[di]=nxt; else orders_get_mut(prev).nextPending=nxt; if(t.pendTail[di]==cur) t.pendTail[di]=prev; }
        else prev=cur; cur=nxt; }
}

static bool compute_transfer(const Train &A,int baseA,int sIdx,int xIdx,const Train &B,int xIdxB,int tIdx,int &baseB, long long &totalTime){
    if (&A==&B) return false; int arrAX=A.arr[xIdx]; int depAS=A.dep[sIdx]; long long absArrA=(long long)baseA*1440 + arrAX; int depBX=B.dep[xIdxB]; int offBX=depBX/1440; int timeBX=depBX%1440; long long arrDayA=absArrA/1440; int arrTimeA=absArrA%1440; long long boardDayB = arrDayA + (timeBX>=arrTimeA?0:1);
    baseB = (int)(boardDayB - offBX); if(baseB < B.saleStart || baseB > B.saleEnd) return false; long long absDepA=(long long)baseA*1440 + depAS; long long absArrB=(long long)baseB*1440 + B.arr[tIdx]; totalTime = absArrB - absDepA; return totalTime>=0; }


static void append_log(const string &line){ FILE *f=fopen("ops.log","ab"); if(!f) return; fwrite(line.c_str(),1,line.size(),f); fwrite("\n",1,1,f); fclose(f);}

static void handle_line(const string &line, bool fromLog){
    if(line.empty()) return; string cmd; { size_t p=line.find(' '); cmd = (p==string::npos)?line:line.substr(0,p); }
    if(cmd=="add_user"){
            string cu=getArg(line,'c'), u=getArg(line,'u'), p=getArg(line,'p'), n=getArg(line,'n'), m=getArg(line,'m'), g=getArg(line,'g');
            if(users_count()==0){ if(u.empty()||p.empty()||n.empty()||m.empty()){ cout<<-1<<'\n'; return;} int ok=users_add(u,p,n,m,10); if(ok!=-1 && !fromLog) append_log(line); cout<<(ok==-1?-1:0)<<'\n'; }
            else{
                int ci=users_find(cu); int ni=users_find(u); if(ci==-1||!users_get(ci).loggedIn||!g.size()||u.empty()||p.empty()||n.empty()||m.empty()||ni!=-1){ cout<<-1<<'\n'; return; }
                if(toInt(g) >= users_get(ci).privilege){ cout<<-1<<'\n'; return; }
                int ok=users_add(u,p,n,m,toInt(g)); if(ok!=-1 && !fromLog) append_log(line); cout<<(ok==-1?-1:0)<<'\n';
            }
        } else if(cmd=="login"){
            string u=getArg(line,'u'), p=getArg(line,'p'); bool ok = (!u.empty() && !p.empty() && users_login(u,p)); cout<<(ok?0:-1)<<'\n';
        } else if(cmd=="logout"){
            string u=getArg(line,'u'); bool ok = (!u.empty() && users_logout(u)); cout<<(ok?0:-1)<<'\n';
        } else if(cmd=="query_profile"){
            string cu=getArg(line,'c'), u=getArg(line,'u'); int ci=users_find(cu), ui=users_find(u); if(ci==-1||ui==-1||!users_get(ci).loggedIn){ cout<<-1<<'\n'; return;} User &C=users_get(ci), &U=users_get(ui); if(!(C.privilege>U.privilege || ci==ui)){ cout<<-1<<'\n'; return;} cout<<U.username<<' '<<U.name<<' '<<U.mail<<' '<<U.privilege<<'\n';
        } else if(cmd=="modify_profile"){
            string cu=getArg(line,'c'), u=getArg(line,'u'); int ci=users_find(cu), ui=users_find(u); if(ci==-1||ui==-1||!users_get(ci).loggedIn){ cout<<-1<<'\n'; return;} User &C=users_get(ci), &U=users_get(ui); if(!(C.privilege>U.privilege || ci==ui)){ cout<<-1<<'\n'; return;} string np=getArg(line,'p'), nn=getArg(line,'n'), nm=getArg(line,'m'), ng=getArg(line,'g'); if(ng.size() && toInt(ng)>=C.privilege){ cout<<-1<<'\n'; return;} if(np.size()) U.password=np; if(nn.size()) U.name=nn; if(nm.size()) U.mail=nm; if(ng.size()) U.privilege=toInt(ng); if(!fromLog) append_log(line); cout<<U.username<<' '<<U.name<<' '<<U.mail<<' '<<U.privilege<<'\n';
        } else if(cmd=="add_train"){
            Train t; t.id=getArg(line,'i'); string sn=getArg(line,'n'), sm=getArg(line,'m'); string ss=getArg(line,'s'), sp=getArg(line,'p'), sx=getArg(line,'x'), st=getArg(line,'t'), so=getArg(line,'o'), sd=getArg(line,'d'); string sy=getArg(line,'y');
            if(t.id.empty()||sn.empty()||sm.empty()||ss.empty()||sp.empty()||sx.empty()||st.empty()||sd.empty()||sy.empty()){ cout<<-1<<'\n'; return; }
            t.stationNum=toInt(sn); t.seatNum=toInt(sm); t.startTime=parseHHMM(sx); t.type=sy[0]; string sdv[3]; split(sd,'|',sdv,3); t.saleStart=dayIndexFromMMDD(sdv[0]); t.saleEnd=dayIndexFromMMDD(sdv[1]);
            string ssv[110]; string spv[110]; string stv[110]; string sov[110]; int ns=split(ss,'|',ssv,110); int np=split(sp,'|',spv,110); int nt=split(st,'|',stv,110); int no=0; if(so=="_") no=0; else no=split(so,'|',sov,110);
            bool ok=true; if(ns!=t.stationNum||np!=t.stationNum-1||nt!=t.stationNum-1||(!(t.stationNum==2?no==0:no==t.stationNum-2))){ ok=false; }
            if(!ok||t.stationNum<2||t.seatNum<=0){ cout<<-1<<'\n'; return; }
            for(int i=1;i<=t.stationNum;++i) t.stations[i]=ssv[i-1];
            for(int i=1;i<=t.stationNum-1;++i){ t.prices[i]=toInt(spv[i-1]); t.travel[i]=toInt(stv[i-1]); }
            for(int i=2;i<=t.stationNum-1;++i){ t.stopover[i-1]= (no?toInt(sov[i-2]):0); }
            int idx=trains_add(t); if(idx!=-1 && !fromLog) append_log(line); cout<<(idx==-1?-1:0)<<'\n';
        } else if(cmd=="release_train"){
            string id=getArg(line,'i'); bool ok = (!id.empty() && trains_release(id)); if(ok && !fromLog) append_log(line); cout<<(ok?0:-1)<<'\n';
        } else if(cmd=="query_train"){
            string id=getArg(line,'i'), d=getArg(line,'d'); int idx=trains_find(id); if(idx==-1){ cout<<-1<<'\n'; return;} Train &t=trains_get(idx); int base=dayIndexFromMMDD(d); if(base<t.saleStart||base>t.saleEnd){ cout<<-1<<'\n'; return; }
            cout<<t.id<<' '<<t.type<<'\n';
            for(int i=1;i<=t.stationNum;++i){
                cout<<t.stations[i]<<' ';
                print_time(t,base,i,true); cout<<" -> "; print_time(t,base,i,false); cout<<' ';
                int price = (i==1?0:sum_prices(t,1,i-1)); cout<<price<<' ';
                if(i==t.stationNum){ cout<<'x'<<'\n'; }
                else{
                    int seat = t.released? seats_min(t,base, i, i) : t.seatNum; cout<<seat<<'\n';
                }
            }
        } else if(cmd=="delete_train"){
            string id=getArg(line,'i'); bool ok = (!id.empty() && trains_delete(id)); if(ok && !fromLog) append_log(line); cout<<(ok?0:-1)<<'\n';
        } else if(cmd=="query_ticket"){
            string s=getArg(line,'s'), tt=getArg(line,'t'), d=getArg(line,'d'); string pp=getArg(line,'p'); bool sortTime = (pp=="time"||pp.empty()); int day=dayIndexFromMMDD(d);
            struct Item{int idx,base,si,ti,price,seat,time;}; Item arr[4000]; int ac=0;
            for(int i=0;i<trains_count();++i){ Train &t=trains_get(i); if(!t.valid||!t.released) continue; int si=station_index(t,s); int ti=station_index(t,tt); if(si==-1||ti==-1||si>=ti) continue; int base; if(!compute_base_start_day_for_boarding(t,si,day,base)) continue; int seat=seats_min(t,base,si,ti-1); int price=sum_prices(t,si,ti-1); int time=(t.arr[ti]-t.dep[si]); if(ac<4000){ arr[ac++]={i,base,si,ti,price,seat,time}; }
            }
            // sort
            auto lessCmp=[&](const Item&a,const Item&b){ if(sortTime){ if(a.time!=b.time) return a.time<b.time; } else { if(a.price!=b.price) return a.price<b.price; } return cmpStr(trains_get(a.idx).id, trains_get(b.idx).id)<0; };
            // simple insertion sort (ac is small)
            for(int i=1;i<ac;++i){ Item key=arr[i]; int j=i-1; while(j>=0 && lessCmp(key,arr[j])){ arr[j+1]=arr[j]; --j;} arr[j+1]=key; }
            cout<<ac<<'\n';
            for(int k=0;k<ac;++k){ Train &t=trains_get(arr[k].idx); cout<<t.id<<' '<<t.stations[arr[k].si]<<' '; print_time(t,arr[k].base,arr[k].si,false); cout<<" -> "<<t.stations[arr[k].ti]<<' '; print_time(t,arr[k].base,arr[k].ti,true); cout<<' '<<arr[k].price<<' '<<arr[k].seat<<'\n'; }
        } else if(cmd=="query_transfer"){
            string s=getArg(line,'s'), tt=getArg(line,'t'), d=getArg(line,'d'); string pp=getArg(line,'p'); bool sortTime = (pp=="time"||pp.empty()); int day=dayIndexFromMMDD(d);
            bool has=false; int bestAi=-1,bestBi=-1,bestBaseA=0,bestBaseB=0,bestSi=0,bestXi=0,bestTi=0; long long bestTime=0; int bestPrice=0; int bestRideA=0;
            for(int ai=0; ai<trains_count(); ++ai){ Train &A=trains_get(ai); if(!A.valid||!A.released) continue; int si=station_index(A,s); if(si==-1) continue; int baseA; if(!compute_base_start_day_for_boarding(A,si,day,baseA)) continue; for(int xi=si+1; xi<=A.stationNum; ++xi){ // transfer station must be before end
                    for(int bi=0; bi<trains_count(); ++bi){ Train &B=trains_get(bi); if(!B.valid||!B.released) continue; if(ai==bi) continue; int xib=station_index(B,A.stations[xi]); int ti=station_index(B,tt); if(xib==-1||ti==-1||xib>=ti) continue; int baseB; long long totTime; if(!compute_transfer(A,baseA,si,xi,B,xib,ti,baseB,totTime)) continue; int priceA=sum_prices(A,si,xi-1); int priceB=sum_prices(B,xib,ti-1); int totalPrice=priceA+priceB; int rideA=A.arr[xi]-A.dep[si];
                        bool better=false;
                        if(!has) better=true;
                        else if(sortTime){
                            if(totTime<bestTime) better=true;
                            else if(totTime==bestTime){
                                if(rideA<bestRideA) better=true;
                                else if(rideA==bestRideA){
                                    int c=cmpStr(A.id, trains_get(bestAi).id);
                                    if(c<0) better=true;
                                    else if(c==0 && cmpStr(B.id, trains_get(bestBi).id)<0) better=true;
                                }
                            }
                        } else {
                            if(totalPrice<bestPrice) better=true;
                            else if(totalPrice==bestPrice){
                                if(rideA<bestRideA) better=true;
                                else if(rideA==bestRideA){
                                    int c=cmpStr(A.id, trains_get(bestAi).id);
                                    if(c<0) better=true;
                                    else if(c==0 && cmpStr(B.id, trains_get(bestBi).id)<0) better=true;
                                }
                            }
                        }
                        if(better){ has=true; bestAi=ai; bestBi=bi; bestBaseA=baseA; bestBaseB=baseB; bestSi=si; bestXi=xi; bestTi=ti; bestTime=totTime; bestPrice=totalPrice; bestRideA=rideA; }
                }
                }

            }
            if(!has){ cout<<0<<'\n'; }
            else{
                Train &A=trains_get(bestAi); Train &B=trains_get(bestBi); int seatA=seats_min(A,bestBaseA,bestSi,bestXi-1); int seatB=seats_min(B,bestBaseB,station_index(B,A.stations[bestXi]),bestTi-1); int priceA=sum_prices(A,bestSi,bestXi-1); int priceB=sum_prices(B,station_index(B,A.stations[bestXi]),bestTi-1);
                cout<<A.id<<' '<<A.stations[bestSi]<<' '; print_time(A,bestBaseA,bestSi,false); cout<<" -> "<<A.stations[bestXi]<<' '; print_time(A,bestBaseA,bestXi,true); cout<<' '<<priceA<<' '<<seatA<<'\n';
                cout<<B.id<<' '<<A.stations[bestXi]<<' '; print_time(B,bestBaseB,station_index(B,A.stations[bestXi]),false); cout<<" -> "<<B.stations[bestTi]<<' '; print_time(B,bestBaseB,bestTi,true); cout<<' '<<priceB<<' '<<seatB<<'\n';
            }
        } else if(cmd=="buy_ticket"){
            string u=getArg(line,'u'), id=getArg(line,'i'), d=getArg(line,'d'), nf=getArg(line,'n'), fs=getArg(line,'f'), ts=getArg(line,'t'); string q=getArg(line,'q'); int ui=users_find(u); if(ui==-1||!users_get(ui).loggedIn){ cout<<-1<<'\n'; return;} int ti=trains_find(id); if(ti==-1){ cout<<-1<<'\n'; return;} Train &tr=trains_get(ti); if(!tr.released){ cout<<-1<<'\n'; return; }
            int si=station_index(tr,fs), ei=station_index(tr,ts); if(si==-1||ei==-1||si>=ei){ cout<<-1<<'\n'; return; } int n=toInt(nf); if(n<=0||n>tr.seatNum){ cout<<-1<<'\n'; return; }
            int day=dayIndexFromMMDD(d); int base; if(!compute_base_start_day_for_boarding(tr,si,day,base)){ cout<<-1<<'\n'; return; }
            int seat=seats_min(tr,base,si,ei-1); int price=sum_prices(tr,si,ei-1)*n;
            if(seat>=n){ seats_add(tr,base,si,ei-1,-n); order_create(ui,ti,si,ei,n,price,day,base,1); if(!fromLog) append_log(line); cout<<price<<'\n'; }
            else if(q=="true"){ if(n>tr.seatNum){ cout<<-1<<'\n'; } else { int oid=order_create(ui,ti,si,ei,n,price,day,base,0); enqueue_pending(tr,base,oid); if(!fromLog) append_log(line); cout<<"queue\n"; } }
            else cout<<-1<<'\n';
        } else if(cmd=="query_order"){
            string u=getArg(line,'u'); int ui=users_find(u); if(ui==-1||!users_get(ui).loggedIn){ cout<<-1<<'\n'; return;} int head=user_order_head(ui); int cnt=0; for(int x=head;x!=-1;x=orders_get(x).next) ++cnt; cout<<cnt<<'\n'; for(int x=head;x!=-1;x=orders_get(x).next){ const Order &o=orders_get(x); const Train &t=trains_get(o.trainIdx); string st = (o.status==1?"success":(o.status==0?"pending":"refunded")); cout<<'['<<st<<"] "<<t.id<<' '<<t.stations[o.fromIdx]<<' '; print_time(t,o.baseStartDay,o.fromIdx,false); cout<<" -> "<<t.stations[o.toIdx]<<' '; print_time(t,o.baseStartDay,o.toIdx,true); cout<<' '<<o.price<<' '<<o.num<<'\n'; }
        } else if(cmd=="refund_ticket"){
            string u=getArg(line,'u'); string nn=getArg(line,'n'); int nth = nn.empty()?1:toInt(nn); int ui=users_find(u); if(ui==-1||!users_get(ui).loggedIn||nth<=0){ cout<<-1<<'\n'; return;} int x=user_order_head(ui); int prev=-1; for(int i=1;i<nth && x!=-1;++i){ prev=x; x=orders_get(x).next; }
            if(x==-1){ cout<<-1<<'\n'; return;} Order &o=orders_get_mut(x); if(o.status==2){ cout<<-1<<'\n'; return;} Train &t=trains_get(o.trainIdx); if(o.status==1){ // success -> refund seats
                seats_add(t,o.baseStartDay,o.fromIdx,o.toIdx-1,o.num);
            } else if(o.status==0) {
                // remove from pending queue
                int di=o.baseStartDay - t.saleStart; int cur=t.pendHead[di], prevq=-1; while(cur!=-1){ if(cur==x){ int nxt=orders_get_mut(cur).nextPending; if(prevq==-1) t.pendHead[di]=nxt; else orders_get_mut(prevq).nextPending=nxt; if(t.pendTail[di]==cur) t.pendTail[di]=prevq; break;} prevq=cur; cur=orders_get(cur).nextPending; }
            }
            o.status=2; if(!fromLog) append_log(line); cout<<0<<'\n';
            if(o.status==2){ process_pending_for_day(t,o.baseStartDay); }

        } else if(cmd=="clean"){
            users_init(); trains_init(); orders_init(50000); if(!fromLog){ remove("ops.log"); } cout<<0<<'\n';
        } else if(cmd=="exit"){
            if(fromLog) return; cout<<"bye\n"; exit(0);
        }
}

static void replay_log(){
    FILE *f=fopen("ops.log","rb"); if(!f) return; std::ofstream devnull("/dev/null"); std::streambuf *oldbuf = cout.rdbuf(devnull.rdbuf());
    string line; int c; while(true){ line.clear(); while((c=fgetc(f))!=EOF){ if(c=='\n') break; line.push_back((char)c);} if(line.size()) handle_line(line,true); if(c==EOF) break; }
    cout.rdbuf(oldbuf); fclose(f);
}

int main(){ios::sync_with_stdio(false);cin.tie(nullptr);
    users_init(); trains_init(); orders_init(50000);
    replay_log();
    string line; while (getline(cin,line)){
        if(line.empty()) continue; if(line[0]=='>'){/*ignore prompt*/}
        handle_line(line,false);
    }
    return 0;
}

