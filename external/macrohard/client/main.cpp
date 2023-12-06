auto sndmsg(char msg[1024], int port) -> int;

auto main() -> int
{
    sndmsg("Hello, World!", 8080);
}
