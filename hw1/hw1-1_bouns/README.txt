我把兩部分的程式寫在一起
main裡面會去call do_process 
然後  用遞迴的方式 重複call do_process 
使得 要執行的程式都會被執行到
程式邊執行的時候
邊把fork過程的pid存進array
然後最後的子孫被執行後
把fork過程印出來

因為是用遞迴去執行
所以
process 會從最後面開始印回來




make檔名為 hw1-1_bonus_makefile

打  make -f hw1-1_bonus_makefile 編譯

執行:

./hw1-1_bonus ./傳入的檔案名


