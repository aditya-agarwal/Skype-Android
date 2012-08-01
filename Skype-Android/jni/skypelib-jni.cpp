    #include <jni.h>
#include "thinclientprotocol.hpp"

static char* TAG = "Skype JNI";

// Wrapper class with JNI callbacks
class ThinClientProtocolJNI : public ThinClientProtocol
{
private:
	JNIEnv* env;
	jobject obj;
public:
	ThinClientProtocolJNI(const char* platformid,const char *uilanguage) :
		ThinClientProtocol(platformid,uilanguage), env(NULL)
	{
	}

	void onConnected() {
		ThinClientProtocol::onConnected();
		if(env) {
			jclass cls = env->GetObjectClass(obj);
			jmethodID mid = env->GetMethodID(cls, "onConnected", "()V");
			if (mid != NULL) {
				 env->CallVoidMethod(obj, mid);
			}
		}
	}

	void onErrorReceived(uint errorcode,const char *errormessage) {
		ThinClientProtocol::onErrorReceived(errorcode, errormessage);
		if(env) {
			jclass cls = env->GetObjectClass(obj);
			jmethodID mid = env->GetMethodID(cls, "onErrorReceived", "(ILjava/lang/String;)V");
			if (mid != NULL) {
				 env->CallVoidMethod(obj, mid, errorcode, env->NewStringUTF(errormessage));
			}
		}
	}
	void onWarningReceived(uint warningcode,const char *warningmessage) {
		ThinClientProtocol::onWarningReceived(warningcode,warningmessage);
		if(env) {
			jclass cls = env->GetObjectClass(obj);
			jmethodID mid = env->GetMethodID(cls, "onWarningReceived", "(ILjava/lang/String;)V");
			if (mid != NULL) {
				 env->CallVoidMethod(obj, mid, warningcode, env->NewStringUTF(warningmessage));
			}
		}
	}
	void onLoggedIn() {
		ThinClientProtocol::onLoggedIn();
		if(env) {
			jclass cls = env->GetObjectClass(obj);
			jmethodID mid = env->GetMethodID(cls, "onLoggedIn", "()V");
			if (mid != NULL) {
				 env->CallVoidMethod(obj, mid);
			}
		}
	}
	void onLoggedOut(TCPROTO::ACCOUNT_LOGOUTREASON reason) {
		ThinClientProtocol::onLoggedOut(reason);
		if(env) {
			jclass cls = env->GetObjectClass(obj);
			jmethodID mid = env->GetMethodID(cls, "onLoggedOut", "()V");
			if (mid != NULL) {
				 env->CallVoidMethod(obj, mid);
			}
		}
	}

	void onBuddyReceived(uint total, uint buddyIndex, const char* name, const char* fullname, uint availability) {
		ThinClientProtocol::onBuddyReceived(total, buddyIndex, name, fullname, availability);
		if(env) {
			jclass cls = env->GetObjectClass(obj);
			jmethodID mid = env->GetMethodID(cls, "onBuddyReceived", "(IIILjava/lang/String;Ljava/lang/String;)V");
			if (mid != NULL) {
				 env->CallVoidMethod(obj, mid, total, buddyIndex, availability,
						 env->NewStringUTF(name), env->NewStringUTF(fullname));
			}
		}
	}

	void onPresenceUpdated(uint buddyIndex, uint availability) {
		ThinClientProtocol::onPresenceUpdated(buddyIndex, availability);
		if(env) {
			jclass cls = env->GetObjectClass(obj);
			jmethodID mid = env->GetMethodID(cls, "onPresenceUpdated", "(II)V");
			if (mid != NULL) {
				 env->CallVoidMethod(obj, mid, buddyIndex, availability);
			}
		}
	}

	void setEnv(JNIEnv* nenv, jobject nobj) { env = nenv; obj = nobj; }
};

static ThinClientProtocolJNI *tCP = NULL;
static uint callMethods[] = {1,2,3};
static uchar systemId[8] = {1,2,3,4,5,6,8,8};
static char* pstnNumber = "+12345678901";


extern "C" {

jint JNI_OnLoad(JavaVM* vm, void* reserved)
{
    return JNI_VERSION_1_4;
}

void JNI_OnUnload(JavaVM* vm, void* reserved)
{
	if (tCP)
		delete tCP;
}

JNIEXPORT void JNICALL Java_com_skype_android_ThinClientProtocol_initialize(
		JNIEnv* env,
		jobject obj)
{
	if (tCP)
		delete tCP;
    tCP = new ThinClientProtocolJNI("1801/1.0.0.1/Reference implementation", "en");
    tCP->SetCapabilities(0);
    tCP->SetCallMethods(callMethods, COUNTOF(callMethods));
    tCP->SetSystemID(systemId);
    tCP->SetPSTNNumber(pstnNumber);
}

JNIEXPORT void JNICALL Java_com_skype_android_ThinClientProtocol_setUsername(
		JNIEnv* env,
		jobject obj,
		jstring username)
{
	const char *usernameChars = env->GetStringUTFChars(username, 0);
	tCP->SetUserName(usernameChars);
	env->ReleaseStringUTFChars(username, usernameChars);
}

JNIEXPORT void JNICALL Java_com_skype_android_ThinClientProtocol_setPassword(
		JNIEnv* env,
		jobject obj,
		jstring password)
{
	const char *passwordChars = env->GetStringUTFChars(password, 0);
	tCP->SetPassword(passwordChars);
	env->ReleaseStringUTFChars(password, passwordChars);
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_login(
		JNIEnv* env,
		jobject obj)
{
	return tCP->Login(0);
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_logout(
		JNIEnv* env,
		jobject obj)
{
	return tCP->Logout();
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_connect(
		JNIEnv* env,
		jobject obj,
		jstring host)
{
	const char *hostChars = env->GetStringUTFChars(host, 0);
	jboolean result = tCP->Connect(hostChars);
	env->ReleaseStringUTFChars(host, hostChars);
	return result;
}

JNIEXPORT void JNICALL Java_com_skype_android_ThinClientProtocol_disconnect(
		JNIEnv* env,
		jobject obj)
{
	tCP->Disconnect();
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_processIncomingData(
		JNIEnv* env,
		jobject obj)
{
	tCP->setEnv(env, obj);
	bool processed = tCP->ProcessIncomingData();
	tCP->setEnv(NULL, NULL);
	return processed;
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_getBuddyList(
		JNIEnv* env,
		jobject obj)
{
	return tCP->SendGetBuddyList();
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_getPresenceUpdate(
		JNIEnv* env,
		jobject obj)
{
	return tCP->SendGetPresenceUpdate();
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_isLoggedIn(
		JNIEnv* env,
		jobject obj)
{
	return tCP->isLoggedIn();
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_isConnected(
		JNIEnv* env,
		jobject obj)
{
	return tCP->isConnected();
}

JNIEXPORT jboolean JNICALL Java_com_skype_android_ThinClientProtocol_isDisconnected(
		JNIEnv* env,
		jobject obj)
{
	return tCP->isDisconnected();
}

}
