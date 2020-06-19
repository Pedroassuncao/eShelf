FROM python:3

WORKDIR /usr/src/app
COPY ./api/requirements.txt ./
RUN pip3 install --no-cache-dir -r requirements.txt

CMD [ "python", "./api.py" ]