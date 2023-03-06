/** 
 * @                       _oo0oo_
 * @                      o8888888o
 * @                      88" . "88
 * @                      (| -_- |)
 * @                      0\  =  /0
 * @                    ___/`---'\___
 * @                  .' \\|     |// '.
 * @                 / \\|||  :  |||// \
 * @                / _||||| -:- |||||- \
 * @               |   | \\\  - /// |   |
 * @               | \_|  ''\---/''  |_/ |
 * @               \  .-\__  '-'  ___/-. /
 * @             ___'. .'  /--.--\  `. .'___
 * @          ."" '<  `.___\_<|>_/___.' >' "".
 * @         | | :  `- \`.;`\ _ /`;.`/ - ` : | |
 * @         \  \ `_.   \_ __\ /__ _/   .-` /  /
 * @     =====`-.____`.___ \_____/___.-`___.-'=====
 * @                       `=---='
 * @
 * @
 * @     ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
 * @
 * @           佛祖保佑     永不宕机     永无BUG
 * @
 * @author: yuyuyuj1e 807152541@qq.com
 * @github: https://github.com/yuyuyuj1e
 * @csdn: https://blog.csdn.net/yuyuyuj1e
 * @date: 2023-02-27 18:25:07
 * @last_edit_time: 2023-03-06 09:51:04
 * @file_path: /Cpp-Web-Server/main.cpp
 * @description: 程序主函数，设置 DEBUG__ 后，可以直接启动程序，无需设置端口以及所需路径，否则需要在可执行文件后添加两个命令行参数
 */

#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include "TcpServer.h"

#define DEBUG__

int main(int argc, const char** argv) {
#ifndef DEBUG__
    if (argc < 3) {
        std::cout << "you need input ./a.out port path\n" << std::endl;
    }

    unsigned short port = atoi(argv[1]);  // 获取端口
    chdir(argv[2]);  // 切换服务器工作路径
#endif // !DEBUG__

#ifdef DEBUG__
    unsigned short port = 10000;  // 获取端口
    chdir("/home/ubuntu/桌面/tt/");  // 切换服务器工作路径
#endif // DEBUG__

    // 启动服务器
    TcpServer* server = new TcpServer(port, 4);
    server->run();
    return 0;
}