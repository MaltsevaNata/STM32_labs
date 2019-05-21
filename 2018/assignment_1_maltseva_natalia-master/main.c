
int N = 5;


int main(void)
{
	volatile int a = 1;
	volatile int b = 1;
	for (int volatile i = 2; i < N; i++) {
			b = a + b;
			a = b - a;
		}
	volatile int answer = b;

  return 0;
}
