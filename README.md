NRF24L01+ Library for any platform
=====
![img](https://ss1.bdstatic.com/70cFvXSh_Q1YnxGkpoWK1HF6hhy/it/u=1421768554,2275816689&fm=26&gp=0.jpg)
> this libarary file is originally developed for STM32's HAL framwork  
> but very easy for you to imagrate it to any other platform(->arm's processor,->Linux)

thank for the healthy state machine `nrf_mode` `nrf_change_device_mode(mod_to_change)` , any step of app do is under monitor.if something unexpected happened , app can know it for the frist time

> (for example , when nrf24l01+ is not in sleep mode or powerdown mode,the action that app's write register will not succeed. but SPI bus will work normally so you can't know that action is fail)

how to use
---------
> init :  
> ~~~c++
> nrf_set_enable(NRF_MODE_Power_Off);
> nrf_config_rx_channel(chn,rf_addr_read_of,pack_size,address_of_your_rx_buff);
> nrf_config_tx_address(rf_addr_write_to);
> nrf_set_enable(NRF_MODE_(RX|TX));
> ~~~

> loop : (or rtos loop,system call.etc)  
> rx:
> ~~~c++
> // check interrupt and read from rx buffer
> ~~~
> tx:
> ~~~c++
> nrf_load_tx(data,size);
> // wait for interrupt to tell whether it's succeed or fail
> ~~~

> interrupt handler :  
> rx : 
> ~~~c++
> int chn = nrf_interrupt_handler();
> if(chn >= 0 && chn <= 5){
>   ... // tell app to read rx buffer;
> }else{
>   ... // un-define error
> }
> ~~~
> tx :
> ~~~c++
> int inf = nrf_interrupt_handler();
> if(inf == 6){
>   ... // success , let app know to trancemit next package
> }else if(inf == 7){
>   ... //retry overtimes
> }else {
>   ... // un-define error
> }
> ~~~

how to imagrate
--------
1,open `nrf24l01p.c`
2,edit defines CSN,CE,delay,write,read

> do not edit any other part of this file

how to develop
---------
<<<<<<< HEAD
> I think , as a developer , you must know it
=======
> I think , as a developer , you must know know it
>>>>>>> 72e80351410d3ad302c0902913c0888f0d65c568
