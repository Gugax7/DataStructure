void inserirfc(int x, int Filac[], int *fim, int inicio) {
	if((*fim + 1) % MAX == inicio) {
		printf("Overflow FC");
	} else {
		*fim = (*fim + 1) % MAX);
		Filac[*fim] = x;
	}
}

void eliminarfc(int *elem, int *inicio, int fim, int Filac[]) {
	if(*inicio == fim) {
		printf("Underflow FC");
	} else {
		*inicio = (*inicio + 1) % MAX);
		*elem = Filac[*inicio];
	}
}
