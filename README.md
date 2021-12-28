# tetris_on_Achro-I.MX6Q
마이크로프로세서응용설계 강의에서 진행한 프로젝트

Achro-I.MX6Q에서 구동되는 테트리스를 구현

게임 시작 방법
1. "make" 실행
2. sudo chmod로 init 실행 가능 권한
3. "./init"
4. "sudo ./tetris"

0 1 2
3 4 5
6 7 8

1: 시계방향으로 회전
3: 좌측으로 1칸 이동
4: Slow down(한칸 떨어지기)
5: 우측으로 1칸 이동
6: 현재 완성한 라인의 수 7 segment에 출력
7: Hard down(떨어질수 있는 만큼 한번에 떨어지기)
8: 현재 레벨 7 segment에 출력
