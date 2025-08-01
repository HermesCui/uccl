// This file is copied from UCCL to avoid the dependency on NCCL.

#include "param.h"
#include <algorithm>
#include <errno.h>
#include <pthread.h>
#include <pwd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <unistd.h>

char const* userHomeDir() {
  struct passwd* pwUser = getpwuid(getuid());
  return pwUser == NULL ? NULL : pwUser->pw_dir;
}

void setEnvFile(char const* fileName) {
  FILE* file = fopen(fileName, "r");
  if (file == NULL) return;

  char* line = NULL;
  char envVar[1024];
  char envValue[1024];
  size_t n = 0;
  ssize_t read;
  while ((read = getline(&line, &n, file)) != -1) {
    if (line[read - 1] == '\n') line[read - 1] = '\0';
    int s = 0;  // Env Var Size
    while (line[s] != '\0' && line[s] != '=') s++;
    if (line[s] == '\0') continue;
    strncpy(envVar, line, std::min(1023, s));
    envVar[std::min(1023, s)] = '\0';
    s++;
    strncpy(envValue, line + s, 1023);
    envValue[1023] = '\0';
    setenv(envVar, envValue, 0);
    // printf("%s : %s->%s\n", fileName, envVar, envValue);
  }
  if (line) free(line);
  fclose(file);
}

static void initEnvFunc() {
  char confFilePath[1024];
  char const* userFile = getenv("UCCL_CONF_FILE");
  if (userFile && strlen(userFile) > 0) {
    snprintf(confFilePath, sizeof(confFilePath), "%s", userFile);
    setEnvFile(confFilePath);
  } else {
    char const* userDir = userHomeDir();
    if (userDir) {
      snprintf(confFilePath, sizeof(confFilePath), "%s/.uccl.conf", userDir);
      setEnvFile(confFilePath);
    }
  }
  snprintf(confFilePath, sizeof(confFilePath), "/etc/uccl.conf");
  setEnvFile(confFilePath);
}

void initEnv() {
  static pthread_once_t once = PTHREAD_ONCE_INIT;
  pthread_once(&once, initEnvFunc);
}

void ucclLoadParam(char const* env, int64_t deftVal, int64_t uninitialized,
                   int64_t* cache) {
  static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;
  pthread_mutex_lock(&mutex);
  if (__atomic_load_n(cache, __ATOMIC_RELAXED) == uninitialized) {
    char const* str = ucclGetEnv(env);
    int64_t value = deftVal;
    if (str && strlen(str) > 0) {
      errno = 0;
      value = strtoll(str, nullptr, 0);
      if (errno) {
        value = deftVal;
        printf("Invalid value %s for %s, using default %lld.\n", str, env,
               (long long)deftVal);
      } else {
        printf("%s set by environment to %lld.\n", env, (long long)value);
      }
    }
    __atomic_store_n(cache, value, __ATOMIC_RELAXED);
  }
  pthread_mutex_unlock(&mutex);
}

char const* ucclGetEnv(char const* name) {
  initEnv();
  return getenv(name);
}
