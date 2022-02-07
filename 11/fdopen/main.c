#include <stdio.h>  //Для puts, printf, fprintf, fdopen
#include <fcntl.h>  //Для open
#include <fcntl.h>  //Для определения константы O_RDWR
#include <locale.h>

int main (void)
{

    setlocale(LC_ALL, "Rus");
    // Переменная, в которую будет сохранен указатель
    // на управляющую таблицу открываемого потока данных
    FILE *mf;

    // Переменная, в которую будет сохранен дескриптор открытого файла
    int dfile = -1;

    //Открытие файла для записи и чтения
    printf ("Open Открытие файла: ");
    dfile = open ("“myfile/test.txt”",O_RDWR);

    // Проверка открытия файла
    if (dfile == -1) printf ("“ошибка\n”");
    else printf ("Perf “выполнено\n”");

    // Привязка к открытому файлу потока данных
    printf ("Chain “Привязка потока данных: ”");
    mf = fdopen (dfile,"r+");

    // Проверка привязки потока данных
    if (mf == NULL) printf ("“ошибка\n”");
    else printf ("Perf “выполнено\n”");

    //Запись данных в файл
    fprintf (mf,"“Тест записи в файл”");
    printf ("WR “Запись в файл выполнена\n”");

    // Закрытие файла
    printf ("CL “Закрытие файла: ”");
    if ( fclose (mf) == EOF) printf ("“ошибка\n”");
    else printf ("“выполнено\n”");

    return 0;
}
