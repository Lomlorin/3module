# 3module
3 модуль. элтекс
//01.с - Задание 01.Написать программу, вычисляющую площади квадратов с заданной длиной стороны. Длины сторон передаются как аргументы запуска. Расчеты делают родительский и дочерний процессы, разделяя задачи примерно поровну. Используем fork(). gss 


//02.c, sum.c -Задание 02. Написать программу, похожую на командный интерпретатор. При запуске выводится приглашение, и пользователь вводит имя и аргументы программы, которую желает запустить. Программы для запуска написать самостоятельно и поместить в тот же каталог (например, программа для вычисления суммы аргументов, «склеивания» строк, поиска наибольшего значения или наибольшей длины строки и т.д.). Для проверки корректности работы программы приложен файл sum.c После компиляции основной программы, программу для запуска нужно запускать в виде "./sum arg arg", что указано при запуске. 


//03.с, 03.txt - Задание 03.Скорректировать решение задачи о книге контактов из модуля 2 так, чтобы список контактов хранился в файле. Использовать небуферизованный ввод-вывод.При запуске программы список считывается из файла,при завершении программы список записывается в файл. Учесть, что при запуске программы может не быть сохраненных данных. 


//04.c 04.txt - Задание 04. Написать программу, порождающую дочерний процесс ииспользующую однонаправленный обмен данными.Процесс-потомок генерирует случайные числа иотправляет родителю. Родитель выводит числа на экран и в файл. Количество чисел задается в параметрах запуска приложения. При запуске сразу вводим нужное кол-во сгенерированных чисел. 


//05.c 05.txt- Задание 05. Изменить программу 04 так, чтобы дочерний процесс выводил информацию из файла. Если родительский процесс собирается изменить файл, то он отправляет сигнал SIGUSR1 (блокировка доступа к файлу). Когда родительский процесс завершил модификацию файла, он отправляет сигнал SIGUSR2 (разрешение доступа к файлу). Дочерний процесс отправляет новое число родительскому после того, как прочитал файл.  
