// Filename: expand.I
// Created by:  cary (20Mar00)
//
////////////////////////////////////////////////////////////////////

INLINE bool Base_Expander::isEnv(ConfigString S) {
  return (getenv(S.c_str()) != (TYPENAME ConfigString::value_type *)0L);
}

INLINE ConfigString Base_Expander::Env(ConfigString S) {
  return isEnv(S)?getenv(S.c_str()):"";
}

INLINE bool Base_Expander::isUser(ConfigString S) {
   if (S.find_first_of(VChars) == 0) {
      std::auto_ptr<struct passwd> pw(getpwnam(S.c_str()));
      return (pw.get() != (struct passwd *)0L);
   } else
      return false;
}

INLINE ConfigString Base_Expander::GetMyDir() {
   std::auto_ptr<struct passwd> pw(getpwuid(geteuid()));
   return pw->pw_dir;
}

INLINE ConfigString Base_Expander::GetUserDir(ConfigString S) {
   std::auto_ptr<struct passwd> pw(getpwnam(S.c_str()));
   return ((pw.get() != (struct passwd *)0L)?pw->pw_dir:"");
}

INLINE ConfigString Base_Expander::operator()(void) {
  return _result;
}

INLINE ConfigString Base_Expander::operator()(ConfigString S) {
  _result = Base_Expander::Expand(S);
  return _result;
}

INLINE Base_Expander::operator ConfigString() {
  return _result;
}

INLINE ConfigString Expand(ConfigString S) {
   Base_Expander ex(S);
   return ex;
}
