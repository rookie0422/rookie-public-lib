# LwIP

[LwIP应用开发实战指南](https://doc.embedfire.com/net/lwip/zh/latest/)

## 1. LwIP简介

> LwIP全名：`Light weight IP`  轻量化的TCP/IP协议
>
> LwIP的设计初衷是：用`少量`的`资源消耗`实现一个`较为完整`的`TCP/IP协议栈`
>
> 其中“`完整`”主要指的是`TCP协议`的完整性，实现的重点是在保持TCP协议主要功能的基础上`减少对RAM 的占用`。
>
> LwIP既可以移植到`操作系统`上运行，也可以在`裸机`的情况下独立运行
>



## 2. 主要特性

> 1. 支持ARP协议（以太网地址解析协议）。
> 2. 支持ICMP协议（控制报文协议），用于网络的调试与维护。
> 3. 支持IGMP协议（互联网组管理协议），可以实现多播数据的接收。
> 4. 支持UDP协议(用户数据报协议)。
> 5. 支持TCP协议(传输控制协议)，包括阻塞控制、RTT 估算、快速恢复和快速转发。
> 6. 支持PPP协议（点对点通信协议），支持PPPoE。
> 7. 支持DNS（域名解析）。
> 8. 支持DHCP协议，动态分配IP地址。
> 9. 支持IP协议，包括IPv4、IPv6协议，支持IP分片与重装功能，多网络接口下的数据包转发。
> 10. 支持SNMP协议（简单网络管理协议）。
> 11. 支持AUTOIP，自动IP地址配置。
> 12. 提供专门的内部回调接口(Raw API)，用于提高应用程序性能。
> 13. 提供可选择的Socket API、NETCONN API (在多线程情况下使用) 。
>



## 3. 优点

> 1.**`资源开销低，即轻量化`**。
>
> LwIP内核有自己的`内存管理策略`和`数据包管理策略`， 使得内核处理数据包的效率很高。
>
> LwIP`高度可剪裁`，一切不需要的功能都可以通过宏编译选项去掉。
>
> LwIP的流畅运行需要`40KB的代码ROM`和`几十KB的RAM`，这让它非常适合用在内存资源受限的嵌入式设备中。
>
> 
>
> 2.**`支持的协议较为完整`**
>
> 几乎支持`TCP/IP`中所有常见的协议，这在嵌入式设备中早已够用。
>
> 
>
> 3.**`实现了一些常见的应用程序`**：
>
> DHCP客户端、DNS客户端、HTTP服务器、MQTT客户端、TFTP服务器、SNTP客户端等等。
>
> 
>
> 4.**同时提供了三种编程接口**：
>
> `RAW/Callback API`、`NETCONN API`、`Socket API`。
>
> 
>
> 5.**`高度可移植`**
>
> 其`源代码`全部用`C语言`实现，用户可以很方便地实现`跨处理器`、`跨编译器`的移植。
>
> 它对内核中会使用到`操作系统功能`的地方进行了`抽象`，使用了一套自定义的`API`，用户可以通过自己实现这些API，从而实现`跨操作系统`的移植工作。
>
> 
>
> 6.**`开源 免费`**



## 4. LwIP文件说明

[LwIP的项目主页](http://savannah.nongnu.org/projects/lwip/)

[LwIP的文件说明](https://doc.embedfire.com/net/lwip/zh/latest/doc/chapter2/chapter2.html)



## 5. LwIP系统框架



### 1.网卡接收数据的流程

开发板上eth`接收完数据`	-->	产生一个`中断`	-->	释放一个`信号量`通知网卡`接收线程`	-->	处理接收数据	-->

将数据`封装成消息`	-->	投递到`tcpip_mbox`邮箱	-->	LwIP`内核`线程得到`消息`	-->	根据消息`数据包类型`进行处理

-->	调用`ethernet_input()`函数决定是否递交到IP层	-->	如果是`ARP`包，内核就不会递交给IP层， 而是`更新ARP缓存表`

​												 -->	`IP数据包`则递交给`IP层`去处理

![图 9‑1网卡数据传入LwIP内核](https://doc.embedfire.com/net/lwip/zh/latest/_images/image145.png)

`用户程序`与`内核`是完全`独立`的，只是通过`操作系统`的`IPC通信`机制进行数据交互。	

`IPC机制`： IPC机制是一种在多个进程之间传输数据或共享信息的机制



### 2. 超时处理

> 在LwIP中，`超时处理`是关键部分，用于管理`ARP缓存`、`IP分片重组`、`TCP连接建立和重传`等。
>
> 旧版本中称为定时器，但在最新版本中，`timer.c`被`timeouts.c`取代。
>
> 超时处理的相关代码实现在`timeouts.c`与`timeouts.h`中。
>
> LwIP没有硬件定时器，而是使用`软件定时器`处理超时，易于维护且与平台无关，只需提供准确的时基。

#### 2.1 sys_timeo结构体与超时链表

> LwIP使用`sys_timeo`结构体来管理所有与超时事件相关的超时链表。
>
> 这些结构体以链表形式连接在一条超时链表中。
>
> 通过一个sys_timeo类型的指针next_timeout，指向当前链表的头部，所有注册的超时事件按处理顺序排列在超时链表上.



```c
typedef void (* sys_timeout_handler)(void *arg);

 struct sys_timeo
 {
     struct sys_timeo *next;                 //指向下一个超时事件的指针，用于超时链表的连接。
     u32_t time;                             //当前超时事件的等待时间。
     sys_timeout_handler h;                  //指向超时的回调函数，该事件超时后就执行对应的回调函数。
     void *arg;                              //向回调函数传入参数。
 };

 /** The one and only timeout list */
 static struct sys_timeo *next_timeout;		 //指向超时链表第一个超时事件。
```



#### 2.2 注册超时事件

> LwIP虽然使用超时链表进行管理所有的`超时事件`，那么它首先需要知道有哪些超时事件才能去管理， 
>
> 而这些超时事件就是通过`注册`的方式被`挂载在链表`上，
>
> 这些超时事件要在`内核`中登记一下， 内核才会去处理，LwIP中注册超时事件的函数是`sys_timeout()`， 但是实际上是调用`sys_timeout_abs()`



```c

void sys_timeout(u32_t msecs, sys_timeout_handler handler, void *arg)
 {
 	 u32_t next_timeout_time;
  	 LWIP_ASSERT_CORE_LOCKED();

 	 /* overflow handled by TIME_LESS_THAN macro */
 	 next_timeout_time = (u32_t)(sys_now() + msecs);    //根据当前时间计算出超时的时间，然后调用sys_timeout_abs()函数将当前事件插入超时链表。

     sys_timeout_abs(next_timeout_time, handler, arg);
 }

 static void sys_timeout_abs(u32_t abs_time, sys_timeout_handler handler, void *arg)
 {
     struct sys_timeo *timeout, *t;

     timeout = (struct sys_timeo *)memp_malloc(MEMP_SYS_TIMEOUT); //内存池中申请一个MEMP_SYS_TIMEOUT类型内存，保存对应超时事件的相关信息。
     if (timeout == NULL)
     {
         return;
     }

     timeout->next = NULL;
     timeout->h = handler;
     timeout->arg = arg;
     timeout->time = abs_time;      //填写对应的超时事件信息，超时回调函数、函数参数、超时的 时间。

     if (next_timeout == NULL)
     {
         next_timeout = timeout;    //如果超时链表中没有超时事件，那么新添加的事件就是链表的第一个。
         return;
     }
     if (TIME_LESS_THAN(timeout->time, next_timeout->time))
     {
         timeout->next = next_timeout;
         next_timeout = timeout;  //如果新插入的超时事件比链表上第一个事件的时间短，则将新插入的超时事件设置成链表的第一个。
     }
     else
     {
         for (t = next_timeout; t != NULL; t = t->next)
         {
             if ((t->next == NULL) ||
                     TIME_LESS_THAN(timeout->time, t->next->time))
             {
                 timeout->next = t->next;
                 t->next = timeout;    //遍历链表，寻找合适的插入节点，超时链表根据超时事件的时间升序排列。
                 break;
             }
         }
     }
 }
```



### 3. tcpip_thread线程

LwIP在操作系统的环境下，LwIP内核是作为`操作系统`的一个线程运行的， 在协议栈初始化的时候就会创建tcpip_thread线程

```c
 static void tcpip_thread(void *arg)
 {
     struct tcpip_msg *msg;
     LWIP_UNUSED_ARG(arg);

     LWIP_MARK_TCPIP_THREAD();

     LOCK_TCPIP_CORE();
     if (tcpip_init_done != NULL)
     {
         tcpip_init_done(tcpip_init_done_arg);
     }

     while (1)
     {
         LWIP_TCPIP_THREAD_ALIVE();
         /* 等待消息，等待时处理超时 */
         TCPIP_MBOX_FETCH(&tcpip_mbox, (void **)&msg); //LwIP将函数tcpip_timeouts_mbox_fetch()定义为带参宏TCPIP_MBOX_FETCH， 所以在这里就															是等待消息并且处理超时事件。
         if (msg == NULL)
         {
             continue;                                //如果没有等到消息就继续等待。
         }
         tcpip_thread_handle_msg(msg);                //等待到消息就对消息进行处理
     }
 }


static void	tcpip_thread_handle_msg(struct tcpip_msg *msg)
 {
     switch (msg->type)
     {
 #if !LWIP_TCPIP_CORE_LOCKING
     case TCPIP_MSG_API:
         msg->msg.api_msg.function(msg->msg.api_msg.msg);  //根据消息中的不同类型进行不同的处理，对于TCPIP_MSG_API类型，就执行对应的API函数。
         break;
     case TCPIP_MSG_API_CALL:
         msg->msg.api_call.arg->err =
             msg->msg.api_call.function(msg->msg.api_call.arg); 
         sys_sem_signal(msg->msg.api_call.sem);
         break;
 #endif /* !LWIP_TCPIP_CORE_LOCKING */

 #if !LWIP_TCPIP_CORE_LOCKING_INPUT
     case TCPIP_MSG_INPKT:		//对于TCPIP_MSG_INPKT类型，直接交给ARP层处理。
     if (msg->msg.inp.input_fn(msg->msg.inp.p, msg->msg.inp.netif) != ERR_OK) 
         {
             pbuf_free(msg->msg.inp.p);
         }
         memp_free(MEMP_TCPIP_MSG_INPKT, msg);
         break;
 #endif /* !LWIP_TCPIP_CORE_LOCKING_INPUT */

 #if LWIP_TCPIP_TIMEOUT && LWIP_TIMERS
     case TCPIP_MSG_TIMEOUT:							//表示上层注册一个超时事件，直接执行注册超时事件即可。
     	 sys_timeout(msg->msg.tmo.msecs, msg->msg.tmo.h, msg->msg.tmo.arg);
         memp_free(MEMP_TCPIP_MSG_API, msg);
         break;
     case TCPIP_MSG_UNTIMEOUT:						//表示上层删除一个超时事件，直接执行删除超时事件即可。
         sys_untimeout(msg->msg.tmo.h, msg->msg.tmo.arg);            
         memp_free(MEMP_TCPIP_MSG_API, msg);
         break;
 #endif /* LWIP_TCPIP_TIMEOUT && LWIP_TIMERS */

     case TCPIP_MSG_CALLBACK:						//表示上层通过回调方式执行一个回调函数，那么就执行对应的回调函数即可。
         msg->msg.cb.function(msg->msg.cb.ctx);                    
         memp_free(MEMP_TCPIP_MSG_API, msg);
         break;

     case TCPIP_MSG_CALLBACK_STATIC:
         msg->msg.cb.function(msg->msg.cb.ctx);                      
         break;

     default:
         break;
     }
 }
```



### 4. LwIP中的消息

#### 4.1 消息结构

LwIP中`消息`是有多种结构的的，对于不同的消息类型其`封装`是不一样的，

`tcpip_thread`线程是通过`tcpip_msg`描述消息的，接收到消息后，根据消息的`类型`进行`不同的处理`。

LwIP中使用`tcpip_msg_type`枚举类型定义了系统中可能出现的消息的类型，

消息结构`msg字段`是一个`共用体`，其中定义了各种消息类型的具体内容，每种类型的消息对应了共用体中的一个字段，其中注册与删除事件的消息使用了同一个tmo字段。

LwIP中的`API`相关的`消息内容很多`，`不适合`直接放在`tcpip_msg`中，所以LwIP用一个`api_msg结构体`来描述`API消息`，在 tcpip_msg中`只存放指向api_msg结构体的指针`

```c
enum tcpip_msg_type
 {
     TCPIP_MSG_API,
     TCPIP_MSG_API_CALL,     //API函数调用
     TCPIP_MSG_INPKT,        //底层数据包输入
     TCPIP_MSG_TIMEOUT,      //注册超时事件
     TCPIP_MSG_UNTIMEOUT,    //删除超时事件
     TCPIP_MSG_CALLBACK,
     TCPIP_MSG_CALLBACK_STATIC       //执行回调函数
 };

 struct tcpip_msg
 {
     enum tcpip_msg_type type;    //消息的类型，目前有7种。
     union
     {
         struct
         {
             tcpip_callback_fn function;	//内核执行的API函数
             void* msg;		//执行函数时候的参数
         } api_msg;     //API消息主要由两部分组成，一部分是用于表示内核执行的API函数，另一部分是执行函数时候的参数，都会被记录在api_msg中。

         struct
         {
             tcpip_api_call_fn function;	//tcpip_api_call_fn类型的函数
             struct tcpip_api_call_data *arg;	//对应的形参
             sys_sem_t *sem;	//用于同步的信号量
         } api_call;   //也是由两部分组成，一部分是tcpip_api_call_fn类型的函数， 另一部分是其对应的形参，此外还有用于同步的信号量。

         struct
         {
             struct pbuf *p;	//p指向接收到的数据包
             struct netif *netif;	//netif表示接收到数据包的网卡
             netif_input_fn input_fn;	// input_fn表示输入的函数接口，在tcpip_inpkt进行配置。
         } inp;   //inp用于记录数据包消息的内容

         struct
         {
             tcpip_callback_fn function;
             void *ctx;
         } cb;       //cb用于记录回调函数与其对应的形参。

         struct
         {
             u32_t msecs;
             sys_timeout_handler h;
             void *arg;
         } tmo;   //tmo用于记录超时相关信息，如超时的时间，超时回调函数，参数等。
     } msg;
 };
```



#### 4.2 数据包消息

对于每种类型的`消息`，LwIP内核都必须有一个产生与之对应的`消息函数`。

在产生该类型的消息后就将其投递到系统邮箱`tcpip_mbox`中， 这样子`tcpip_thread`线程就会从邮箱中得到消息并且`处理`，从而能使内核完美运作，

数据包的消息是通过`tcpip_input()`函数`对消息进行构造并且投递`的， 但是真正执行这些`操作`的函数是`tcpip_inpkt()`



```c
err_t tcpip_input(struct pbuf *p, struct netif *inp)
 {
     if (inp->flags & (NETIF_FLAG_ETHARP | NETIF_FLAG_ETHERNET))
     {
         //调用tcpip_inpkt()函数将ethernet_input()函数作为结构体的一部分传递给内核， 然后内核接收到这个数据包就调用该函数。
         return tcpip_inpkt(p, inp, ethernet_input); 
     }
 }

 err_t tcpip_inpkt(struct pbuf *p, struct netif *inp, netif_input_fn input_fn)
 {
     struct tcpip_msg *msg;

     LWIP_ASSERT("Invalid mbox", sys_mbox_valid_val(tcpip_mbox));

     msg = (struct tcpip_msg *)memp_malloc(MEMP_TCPIP_MSG_INPKT); //申请存放消息的内存空间
     if (msg == NULL)
     {
         return ERR_MEM;
     }
	 //构造消息
     msg->type = TCPIP_MSG_INPKT;	//消息的类型是数据包消息
     msg->msg.inp.p = p;	//初始化消息结构中msg共用体的inp字段， p指向数据包
     msg->msg.inp.netif = inp;	//网卡就是对应的网卡
     msg->msg.inp.input_fn = input_fn;   //处理的函数ethernet_input()函数。
     if (sys_mbox_trypost(&tcpip_mbox, msg) != ERR_OK)  //构造消息完成，就调用sys_mbox_trypost进行投递消息
     {
         memp_free(MEMP_TCPIP_MSG_INPKT, msg);       //如果投递失败，就释放对应的消息结构空间。
         return ERR_MEM;
     }
     return ERR_OK;
 }
```



![图 9‑3数据包消息运作](https://doc.embedfire.com/net/lwip/zh/latest/_images/image318.png)

#### 4.3 API消息

LwIP使用`api_msg`结构体`描述`一个`API消息`的内容

api_msg只包含3个字段：

描述连接信息的`conn` ：保存了`当前连接`的重要信息，如信号量、邮箱等，`lwip_netconn_do_xxx`类型的函数执行需要用这些信息来完成与应用线程的通信与同步

内核返回的`执行结果err` ： 内核执行`lwip_netconn_do_xxx`类型的函数`返回结果`会被记录在err中

`msg` : msg是一个共用体，根据不一样的`API接口`使用不一样的`数据结构`,记录各个函数执行时需要的详细参数。



对于`上层`的API函数，想要与`内核`进行数据`交互`，也是通过LwIP的`消息机制`

`API消息`由`用户线程`发出，与内核进行交互，因为用户的`应用程序`并不是与内核处于同一线程中

简单来说就是用户使用NETCONN API接口的时候，LwIP会将对应`API函数与参数`构造成`消息`传递到`tcpip_thread`线程中，

然后根据对应的API函数`执行`对应的操作，LwIP这样子处理是为了简单用户的编程，这样子就不要求用户对内核很熟悉，

与`数据包消息`类似，也是有独立的`API消息投递函数`去处理，在NETCONN API中构造完成数据包，就会调用`netconn_apimsg()`函数进行投递消息



```c
struct api_msg {
  /** 要处理的netconn——总是需要:它包括信号量
它用于阻塞应用程序线程，直到函数完成。 */
  struct netconn *conn;
  /** tcpip_thread中执行的函数的返回值。 */
  err_t err;
  /** 取决于执行的函数，会使用这些union成员中的一个 */
  union {
    /** 用于lwip_netconn_do_send */
    struct netbuf *b;
    /** 用于 lwip_netconn_do_newconn */
    struct {
      u8_t proto;
    } n;
    /** 用于 lwip_netconn_do_bind and lwip_netconn_do_connect */
    struct {
      API_MSG_M_DEF_C(ip_addr_t, ipaddr);
      u16_t port;
      u8_t if_idx;
    } bc;
    /** 用于 lwip_netconn_do_getaddr */
    struct {
      ip_addr_t API_MSG_M_DEF(ipaddr);
      u16_t API_MSG_M_DEF(port);
      u8_t local;
    } ad;
    /** 用于 lwip_netconn_do_write */
    struct {
      /** current vector to write */
      const struct netvector *vector;
      /** number of unwritten vectors */
      u16_t vector_cnt;
      /** offset into current vector */
      size_t vector_off;
      /** total length across vectors */
      size_t len;
      /** offset into total length/output of bytes written when err == ERR_OK */
      size_t offset;
      u8_t apiflags;
#if LWIP_SO_SNDTIMEO
      u32_t time_started;
#endif /* LWIP_SO_SNDTIMEO */
    } w;
    /** 用于 lwip_netconn_do_recv */
    struct {
      size_t len;
    } r;
#if LWIP_TCP
    /** 用于 lwip_netconn_do_close (/shutdown) */
    struct {
      u8_t shut;
#if LWIP_SO_SNDTIMEO || LWIP_SO_LINGER
      u32_t time_started;
#else /* LWIP_SO_SNDTIMEO || LWIP_SO_LINGER */
      u8_t polls_left;
#endif /* LWIP_SO_SNDTIMEO || LWIP_SO_LINGER */
    } sd;
#endif /* LWIP_TCP */
#if LWIP_IGMP || (LWIP_IPV6 && LWIP_IPV6_MLD)
    /** 用于 lwip_netconn_do_join_leave_group */
    struct {
      API_MSG_M_DEF_C(ip_addr_t, multiaddr);
      API_MSG_M_DEF_C(ip_addr_t, netif_addr);
      u8_t if_idx;
      enum netconn_igmp join_or_leave;
    } jl;
#endif /* LWIP_IGMP || (LWIP_IPV6 && LWIP_IPV6_MLD) */
#if TCP_LISTEN_BACKLOG
    struct {
      u8_t backlog;
    } lb;
#endif /* TCP_LISTEN_BACKLOG */
  } msg;
#if LWIP_NETCONN_SEM_PER_THREAD
  sys_sem_t* op_completed_sem;
#endif /* LWIP_NETCONN_SEM_PER_THREAD */
};
```



### 5 LwIP的三种编程接口



#### 5.1 RAW/Callback API



`RAW/Callback API`是指`内核回调型`的API，这在许多通信协议的C语言实现中都有所应用。

在没有操作系统支持的`裸机环境`中，`只能`使用这种API进行开发，同时这种API`也可以`用在`操作系统`环境中。



**`回调`**：

你`新建`了一个TCP或者UDP的`连接`，想等它接收到`数据`以后去`处理`它们，

这时你需要把`处理该数据的操作封装成一个函数`，然后将这个函数的`指针`注册到`LwIP内核`中。

LwIP内核会在需要的时候去`检测`该连接是否收到数据，如果收到了数据，内核会在第一时间`调用`注册的函数，

这个过程被称为“`回调`”，这个注册函数被称为“`回调函数`”。

这个回调函数中装着你想要的`业务逻辑`，可以自由地处理接收到的数据，也可以发送任何数据，也就是说，这个回调函数就是你的应用程序。



`回调编程`中，LwIP内核把数据交给应用程序的过程就只是`一次简单的函数调用`，这是非常节省时间和空间资源的。

每一个回调函数实际上只是一个普通的C函数，这个函数在TCP/IP内核中被调用。



每一个`回调函数`都作为一个`参数传递给当前TCP或UDP连接`。

为了能够保存程序的特定状态，可以向回调函数传递一个指定的状态，并且这个指定的状态是独立于TCP/IP协议栈的。



有`操作系统`的环境中，如果使用`RAW/Callback API`，用户的`应用程序`就以回调函数的形式成为了`内核代码`的一部分

用户应用程序和内核程序会处于`同一个线程`之中，这就省去了任务间通信和切换任务的开销了。



优点：

1.可以在没有操作系统的环境中使用。

2.在有操作系统的环境中使用它，对比另外两种API，可以提高应用程序的`效率`、`节省内存`开销。



缺点：

1.基于回调函数开发应用程序时的思维过程比较`复杂`。利用回调函数去实现复杂的业务逻辑时，会很麻烦，而且代码的`可读性较差`。

2.在操作系统环境中，应用程序代码与内核代码处于同一个线程，相应地，`应用程序的执行会制约内核程序的执行`，不同的应用程序之间也会互相制约。

在`应用程序执行`的过程中，内核程序将不可能得到运行，这会`影响网络数据包的处理效率`

如果应用程序`占用的时间过长`，而且碰巧这时又有`大量的数据包到达`，由于`内核`代码长期`得不到执行`，网卡接收缓存里的`数据包`就`持续积累`，从而造成`丢包`的现象。







#### 5.2 NETCONN API



`NETCONN API`是基于`操作系统`的IPC机制（即`信号量和邮箱机制`）实现的

它的设计将`LwIP内核代码`和网络`应用程序`分离成了`独立的线程`

LwIP`内核线程`就只负责数据包的TCP/IP`封装和拆封`，而`不用`进行数据的`应用层处理`，大大提高了系统对网络数据包的处理效率。



在操作系统环境中，`LwIP内核`会被实现为一个`独立的线程`，名为`tcpip_thread`

使用NETCONN API或者Socket API的应用程序处在不同的线程中，我们可以根据任务的重要性，分配不同的优先级给这些线程，从而保证重要任务的时效性



**线程优先级分配原则**

| 线程                                       | 优先级 |
| ------------------------------------------ | ------ |
| LwIP内核线程tcpip_thread                   | 很高   |
| 重要的网络应用程序                         | 高     |
| 不太重要而且处理数据比较耗时的网络应用程序 | 低     |



`NETCONN API`使用了操作系统的`IPC`机制，对网络连接进行了`抽象`，用户可以像操作文件一样操作网络连接（`打开/关闭`、`读/写数据`）。



相较于RAW/Callback API，NETCONN API`简化`了`编程`工作，使用户可以按照操作文件的方式来操作网络连接。



`内核程序`和`网络应用程序`之间的`数据包传递`，需要依靠操作系统的`信号量`和`邮箱`机制完成，

这需要`耗费`更多的`时间和内存`，另外还要加上`任务切换`的时间开销，`效率较低`。



NETCONN API`避免`了内核程序和网络应用程序之间的`数据拷贝`，提高了`数据递交`的`效率`。

NETCONN API的`易用性`不如Socket API好，它需要用户对LwIP内核所使用`数据结构`有一定的`了解`。





#### 5.3 SOCKET API



`Socket`，即`套接字`，它对网络连接进行了`高级的抽象`，使得用户可以像操作文件一样操作网络连接。

Socket已经成为了`网络编程`的`标准`。在不同的系统中，运行着不同的TCP/IP协议

只要它实现了Socket的接口，那么用Socket编写的网络应用程序就能在其中运行。可见用Socket编写的网络应用程序具有很好的`可移植性`。





不同的系统有自己的一套Socket接口。

`Windows系统`中支持的是`WinSock`，`UNIX/Linux`系统中支持的是`BSD Socket`，它们虽然风格不一致，但大同小异。

`LwIP`中的Socket API是`BSD Socket`。但是LwIP并没有也没办法实现全部的BSD Socket，

如果开发人员想要移植UNIX/Linux系统中的网络应用程序到使用LwIP的系统中，就要注意这一点。



相较于NETCONN API， Socket API具有更好的`易用性`。使用Socket API编写的程序`可读性好`，便于`维护`，也便于`移植`到其它的系统中。

Socket API在内核程序和应用程序之间`存在数据拷贝`，这会`降低`数据递交的`效率`。

LwIP的Socket API是`基于NETCONN API`实现的，所以效率上相较前者要打个折扣。











## 6. ARP协议

`ARP`：地址解析协议（Address Resolution Protocol）

解析`IP地址`得到`数据链路层地址`，是一个在网络协议包中极其重要的网络传输协议

在**`网络层`**看来，`源主机`与`目标主机`是通过`IP地址`进行识别的

而所有的`数据传输`又依赖网卡`底层硬件`，即链路层，就需要将这些`IP地址`转换为链路层`可识别`的`MAC地址`

“地址解析”就是主机在发送帧前将`目标IP地址`转换成`目标MAC地址`的过程

ARP协议的基本功能就是通过目标设备的IP地址，查询目标设备的MAC地址

### 1. 以太网帧结构

以太网的48位MAC地址存储在网卡内部存储器中。

以太网帧结构:

![图 10‑1以太网帧结构](https://doc.embedfire.com/net/lwip/zh/latest/_images/image11.png)

目标MAC地址（6字节）：

这个字段包含目标网卡的MAC地址。

当一个网卡收到一个以太网数据帧后，只有帧中目标地址是网卡本身的MAC地址或者广播MAC地址，才会将数据传递给网络层，否则将丢弃该数据帧。



源MAC地址（6字节）：

这个字段包含了传输该帧到局域网上的适配器的MAC地址。



类型字段（2字节）：

主机能够使用除了IP以外的其他网络层协议。

当以太网帧到达网卡中，网卡需要知道它应该将数据字段的内容传递给哪个网络层协议。

当这个字段的值小于1518时，它表示后面数据字段的数据长度，当大于1518的时候才表示递交给哪个协议。



数据字段（46~1500字节）：

这个字段承载了IP数据报。



在以太网帧中，目标MAC地址可以分成三类，单播地址、多播地址和广播地址。

单播地址通常是与某个网卡的MAC地址对应，它要求以太网第一个字节的bit0（最先发出去的位）必须是0。

而多播地址则要求第一个字节的bit0为1，这样子多播地址就不会与任何网卡的MAC地址相同，可以被多个网卡同时接收。

广播地址的48位MAC地址全为1，也就是FF-FF-FF-FF-FF-FF， 同一局域网内的所有网卡都会收到广播的数据包。



### 2.  ARP缓存表

TCP/IP协议有自己的`IP地址`，IP地址（IPv4）是一个32位的IP地址。

`网络层`发送数据包只需要知道`目标主机IP地址`即可

而`以太网`发送数据则必须知道对方的`硬件MAC地址`，同时IP地址的分配与硬件MAC地址是`无关`的

为了让网络层只需要知道IP地址就可以完成通信工作，那就需要有一个`协议`将IP地址`映射`成为对应的MAC地址

所以ARP协议就提供优质的地址动态解析的机制，让32位的`IP地址`能`映射`成为48位的`MAC地址`

让`上层应用`于`底层`完全`分离`开，这样子在上层应用就能灵活使用IP地址作为标识，进行通信。

为了实现IP地址与网卡MAC地址的查询与转换，ARP协议引入了ARP缓存表的概念.

每台主机或路由器在其内存中具有一个`ARP缓存表`（ARP table），这张表包含IP地址到MAC地址的映射关系

表中记录了`<IP地址，MAC地址>`对，它们是主机最近运行时获得关于其他主机的`IP地址到物理地址的映射`



ARP协议的核心就是对缓存表的操作

发送数据包的时候，查找ARP缓存表以得到对应的MAC地址，必要时进行ARP缓存表的更新

ARP还需要不断处理其他主机的ARP请求，在ARP缓存表中的TTL即将过期的时候更新缓存表以保证缓存表中的表项有效。



ARP协议的核心是ARP缓存表，ARP的实质就是对缓存表的建立、更新、查询等操作， ARP缓存表的核心是表项.

LwIP使用一个`arp_table`数组描述ARP缓存表， 数组的内容是表项的内容

每个表项都必须记录一对IP地址与MAC地址的映射关系

此外还有一些基本的信息， 如表项的状态、生命周期（生存时间）以及对应网卡的基本信息

LwIP使用一个`etharp_entry`结构体对表项进行描述

```c
//ARP_TABLE_SIZE默认为10，也就是最大能存放10个表项，由于这个表很小，LwIP对表的操作直接采用遍历方式
static struct etharp_entry arp_table[ARP_TABLE_SIZE];


 struct etharp_q_entry
 {
     struct etharp_q_entry *next;
     struct pbuf *p;
 };


 struct etharp_entry
 {
 #if ARP_QUEUEING
     /** 指向此ARP表项上挂起的数据包队列的指针。 */
     //etharp_q_entry指向的是数据包缓存队列， etharp_q_entry是一个结构体
     struct etharp_q_entry *q; 
 #else
     /** 指向此ARP表项上的单个挂起数据包的指针。 */
     struct pbuf *q;   //q直接指向单个数据包
 #endif
     ip4_addr_t ipaddr;      	//记录目标IP地址。
     struct netif *netif;   	//对应网卡信息。
     struct eth_addr ethaddr;  	//记录与目标IP地址对应的MAC地址。
     u16_t ctime;           	//生存时间。
     u8_t state;               //表项的状态，LwIP中用枚举类型定义了不同的状态
 };

 /** ARP states */
 enum etharp_state
 {
     ETHARP_STATE_EMPTY = 0,
     ETHARP_STATE_PENDING,
     ETHARP_STATE_STABLE,
     ETHARP_STATE_STABLE_REREQUESTING_1,
     ETHARP_STATE_STABLE_REREQUESTING_2
 #if ETHARP_SUPPORT_STATIC_ENTRIES
     , ETHARP_STATE_STATIC
 #endif /* ETHARP_SUPPORT_STATIC_ENTRIES */
 };
```

 ARP表项上的单个挂起数据包：

![图 10‑3 ARP表项上的单个挂起数据包](https://doc.embedfire.com/net/lwip/zh/latest/_images/image3.png)



ARP表项上挂起的数据包队列：

![图 10‑4 ARP表项上挂起的数据包队列](https://doc.embedfire.com/net/lwip/zh/latest/_images/image4.png)

### 3.  ARP超时处理

ARP是动态处理的，表项的生存时间是5分钟，而ARP请求的等待时间是5秒钟

当这些时间到达后，就会更新ARP表项，如果在物理链路层无法连通则会删除表项。

需要ARP层有一个超时处理函数对ARP进行管理

这些操作都是根据ARP表项的ctime字段进行的，它记录着对应表项的生存时间

超时处理函数是`etharp_tmr()`

它是一个周期性的超时处理函数，每隔1秒就调用一次，当ctime的值大于指定的时间，就会删除对应的表项

```c
void
 etharp_tmr(void)
 {
     int i;

     LWIP_DEBUGF(ETHARP_DEBUG, ("etharp_timer\n"));
     /* 遍历ARP表，从ARP表中删除过期的表项 */
    //由于LwIP的ARP表是比较小的，直接遍历表即可，更新ARP表的内容。
     for (i = 0; i < ARP_TABLE_SIZE; ++i) 
     {
         u8_t state = arp_table[i].state;
         if (state != ETHARP_STATE_EMPTY
 #if ETHARP_SUPPORT_STATIC_ENTRIES
                 && (state != ETHARP_STATE_STATIC)
 #endif /* ETHARP_SUPPORT_STATIC_ENTRIES */
         )
         {
             arp_table[i].ctime++;  //如果ARP表项不是空的，那么就记录表项的时间。
             if ((arp_table[i].ctime >= ARP_MAXAGE) ||
                     ((arp_table[i].state == ETHARP_STATE_PENDING)  &&
                     (arp_table[i].ctime >= ARP_MAXPENDING))) 
             {
                 /* 等待表项稳定或者表项已经过期*/
         LWIP_DEBUGF(ETHARP_DEBUG,("etharp_timer: expired %s entry %d.\n",
             arp_table[i].state >= ETHARP_STATE_STABLE ? "stable" : "pending", i));
                 /*从ARP表中删除过期的表项 */
                 etharp_free_entry(i);    
/**                 
当表项的时间大于表项的生存时间（5分钟）， 或者表项状态是ETHARP_STATE_PENDING处于等待目标主机回应ARP请求包， 并且等待的时间超过ARP_MAXPENDING（5秒），那么LwIP就认为这些表项是无效了， 就调用etharp_free_entry()函数删除表项。**/
                 
             }
             else if (arp_table[i].state == ETHARP_STATE_STABLE_REREQUESTING_1)
             {
                 /* 过渡 */
                 arp_table[i].state = ETHARP_STATE_STABLE_REREQUESTING_2;
             }
         else if (arp_table[i].state == ETHARP_STATE_STABLE_REREQUESTING_2)
             {
         /* 进入ETHARP_STATE_STABLE状态 */

                 arp_table[i].state = ETHARP_STATE_STABLE;
             }
             else if (arp_table[i].state == ETHARP_STATE_PENDING)
             {
                 /*仍然挂起，重新发送ARP请求 */
                 etharp_request(arp_table[i].netif, &arp_table[i].ipaddr);
             }
         }
     }
 }
```



### 4. ARP报文

ARP的请求与应答都是依赖ARP报文结构进行的

ARP报文是放在以太网数据帧中进行发送的

[APR报文具体工作流程](https://zhuanlan.zhihu.com/p/395157603)





## 7. IP协议

`IP协议`（Internet Protocol），又称之为`网际协议`，是整个TCP/IP协议栈的`核心`协议

IP协议负责将`数据报`从源主机`发送`到目标主机，通过IP地址作为`唯一识别码`

不同主机之间的IP地址是不一样的

IP协议还可能对数据报进行`分片处理`，同时在接收数据报的时候还可能需要对分片的数据报进行重装等等

IP协议是一种`无连接`的`不可靠`数据报交付协议，协议本身不提供任何的错误检查与恢复机制。



### 1. IP地址

为了标识互联网中的每台主机的身份，每个接入网络中的主机都分配一个IP地址，是一个32位的整数地址

IP地址是软件地址，不是硬件地址，硬件MAC地址是存储在网卡中的，应用于本地网络中寻找目标主机

IP地址能让一个网络中的主机能够与另一个网络中的主机进行通信，与主机之间的MAC地址无关。











































































