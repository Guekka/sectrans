#ifdef __cplusplus
extern "C"
{
#endif
    auto startserver(int port) -> int;
    auto stopserver() -> int;

    /* read message sent by client */
    auto getmsg(char msg_read[1024]) -> int;
#ifdef __cplusplus
}
#endif
