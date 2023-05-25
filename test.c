int x;
const int y = 10;
const int z = y + 1;
int init = 1;

int main() {
  int i = 0;
  while (i < 10) {
    i = 20;
    break;
    i = i + 1;
  }
  return i;
}