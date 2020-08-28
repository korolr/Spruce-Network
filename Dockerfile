FROM gcc:10

COPY . /src
WORKDIR /src
RUN apt-get update && apt-get upgrade
RUN apt-get -y install libsodium-dev sqlite3 php
RUN make