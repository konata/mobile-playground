#include <cn_beriru_jni_CallJniActivity.h>
#include "sstream"

using namespace std;

const char* TAG = __FILE__ ;


JNIEXPORT jstring JNICALL Java_cn_beriru_jni_CallJniActivity_concat (JNIEnv * env,jobject self,jstring str)
{
	const char* s = _s(env,str);
	string jniStr = "string from jni";
	return env->NewStringUTF(jniStr.append(s).c_str());
}



JNIEXPORT jint JNICALL Java_cn_beriru_jni_CallJniActivity_initWatchDog(JNIEnv *env, jobject obj)
{
	int child_pid = init_watchdog();
	return child_pid;
}


const char* _s(JNIEnv* env, jstring str)
{
	const char* ret = env->GetStringUTFChars(str,0);
	return ret;
}


void loge(const string& content)
{
	__android_log_write(ANDROID_LOG_ERROR, TAG, content.c_str());
}

void panic(const string& s)
{
	loge("panic : " + s);
}

int init_watchdog()
{
	string monitor_dir = "/data/data/cn.beriru.playground";

	pid_t pid = fork();

	if(pid < 0){
		panic("fork error");
	}else if(pid == 0){
		loge("in child process");
		child_process(monitor_dir);
		return 0;
	}else{
		stringstream s;
		s <<  "in parent process child id is ";
		s << pid;
		loge(s.str().c_str());
		waitpid(-1,NULL,WNOHANG);
		return pid;
	}
}


void child_process(string monitor_dir)
{
	struct pollfd dog;
	struct pollfd dog_list[] = { dog };

	dog.fd = inotify_init();
	dog.events = POLLIN;
	int watch_ret = inotify_add_watch(dog.fd,monitor_dir.c_str(),IN_DELETE);
	if(watch_ret < 0){
		panic("error watch");
		return ;
	}

	loge("enter loop");

	while(true){
		loge("enter loop");
		int ret = poll(dog_list,(unsigned long) 1, -1);
		if(ret < 0){
			panic("poll error");
			break;
		}else{
			if((dog.revents & POLLIN) == POLLIN){
				loge("pollin event!");
				if(access(monitor_dir.c_str(),F_OK) != 0){
					sleep(2);
					string cmd = "am start -a android.intent.action.VIEW -d http://konata.github.io ";
					string cmd_with_init = "am start --user 0 -a android.intent.action.VIEW -d http://konata.github.io ";
					system(cmd.c_str());
					system(cmd_with_init.c_str());
					loge("byebye");
					exit(0);
				}else{
					loge("relisten dir");
					inotify_rm_watch(dog.fd, watch_ret);
					dog.fd = inotify_init();
					dog.events = POLLIN;
					watch_ret = inotify_add_watch(dog.fd,monitor_dir.c_str(),IN_CREATE);
				}
			}else{
				loge("returned!!!");
			}
		}
	}

	inotify_rm_watch(dog.fd,watch_ret);
	return ;
}

