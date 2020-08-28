FROM gcc:10

COPY . /src
WORKDIR /src
RUN apt-get update && upgrade
RUN apt-get install sodium sqlite3 php
RUN make