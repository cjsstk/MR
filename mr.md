---
description: "지라 티켓으로 feature 브랜치 push 후 GitLab MR 링크 출력"
---

입력 형식: `{지라티켓} [{참고파일.md}]`
예: `QSEV-51747` 또는 `QSEV-51747 .claude/docs/task/20260421_QSEV-51747.md`

아래 절차를 순서대로 실행해:

1. `$ARGUMENTS`를 파싱해 첫 번째 토큰을 JIRA_TICKET, 나머지를 DOC_FILE로 분리

2. staged 파일이 있으면 커밋:
   - `git diff --cached --name-only`로 staged 파일 확인
   - staged 파일이 있으면:
     a. Atlassian MCP(`getJiraIssue`)로 JIRA_TICKET의 summary 필드 조회 (cloudId: `74455258-b260-4af0-bf07-4de7a01f2fdf`)
     b. `git commit -m "{JIRA_TICKET} {summary}"` 로 커밋
   - staged 파일이 없으면 이 단계 건너뜀

3. git remote URL에서 GitLab host/project 추출:
   ```bash
   git remote get-url origin
   # git@gitlab:q7/game.git → host=gitlab, project=q7/game
   ```

4. git config에서 사용자 이름 추출:
   ```bash
   git config user.name | tr '[:upper:]' '[:lower:]' | awk -F'-' '{print $NF}'
   # AnKwangJun-Bop → bop
   ```

5. 브랜치명: `{username}/{JIRA_TICKET}`

6. 원격 브랜치 존재 여부 확인 후 push:
   ```bash
   git ls-remote --heads origin {username}/{JIRA_TICKET}
   ```
   - 결과가 비어있으면(신규): `git push origin master:{username}/{JIRA_TICKET}`, IS_FORCE_PUSH=false
   - 결과가 있으면(기존): `git push -f origin master:{username}/{JIRA_TICKET}`, IS_FORCE_PUSH=true

7. description 구성:
   - DOC_FILE이 지정된 경우: 해당 파일을 읽어 핵심 작업 내용만 2~5줄로 요약
   - DOC_FILE이 없는 경우: `git log --oneline origin/master..HEAD 2>/dev/null || git log --oneline -5` 결과 사용
   
   description 형식:
   ```
   ## 작업 내용
   * {요약 항목 1}
   * {요약 항목 2}
   ...

   @q7-client
   ```

8. 브라우저 열기:
   - IS_FORCE_PUSH가 false(신규 MR)인 경우: description을 python3으로 URL 인코딩 후 MR 생성 페이지 열기:
     ```bash
     DESC=$(python3 -c "import urllib.parse; print(urllib.parse.quote(open('...').read() if ... else '...'))")
     start "http://{host}/{project}/-/merge_requests/new?merge_request[source_branch]={username}/{JIRA_TICKET}&merge_request[target_branch]=master&merge_request[assignee_ids][]=524&merge_request[force_remove_source_branch]=1&merge_request[description]=$DESC"
     ```
   - IS_FORCE_PUSH가 true(기존 MR 업데이트)인 경우: 기존 MR 목록 페이지 열기:
     ```bash
     start "http://{host}/{project}/-/merge_requests?scope=all&state=opened&search={JIRA_TICKET}"
     ```

push 결과와 열린 MR 링크를 출력해. push 실패 시 에러 메시지를 그대로 출력해.