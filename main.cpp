#define _CRT_SECURE_NO_WARNINGS
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>
#include <unordered_map>

struct prevkeys
{
	int a, b, c, d, e, f;
	prevkeys()
	{
		reset();
	}
	void reset()
	{
		a = b = c = d = e = f = -1;
	}
	void shift()
	{
		f = e;
		e = d;
		d = c;
		c = b;
		b = a;
		a = -1;
	}
	int depth()
	{
		if (f != -1) return 6;
		if (e != -1) return 5;
		if (d != -1) return 4;
		if (c != -1) return 3;
		if (b != -1) return 2;
		if (a != -1) return 1;
		return 0;
	}
	void sink()
	{
		if (f != -1)
		{
			f = -1;
		}
		else
		if (e != -1)
		{
			e = -1;
		}
		else
		if (d != -1)
		{
			d = -1;
		}
		else
		if (c != -1)
		{
			c = -1;
		}
		else
		if (b != -1)
		{
			b = -1;
		}
		else
		{
			a = -1;
		}
	}
	bool operator==(const prevkeys &other) const
	{
		return (a == other.a) && (b == other.b) && (c == other.c) && (d == other.d) && (e == other.e) && (f == other.f);
	}
};

namespace std {

	template <>
	struct hash<prevkeys>
	{
		std::size_t operator()(const prevkeys& k) const
		{
			return (k.a << 1)^(k.b << 3)^(k.c << 2)^(k.d)^(k.e << 3)^(k.f << 1);
		}
	};

}

int whitespace(char aCharacter)
{
	switch (aCharacter)
	{
	case ' ':
	case '\t':
	case '\r':
	case '\n':
		return 1;
	}
	return 0;
}

int is_numeric(char *aString)
{
	if (!*aString) return 0;
	while (*aString)
	{
		if (*aString < '0' || *aString > '9')
			return 0;
		aString++;
	}
	return 1;
}

int is_alpha(char *aChar)
{
	if (!*aChar) return 0;
	if (toupper(*aChar) < 'A' || toupper(*aChar) > 'Z')
		return 0;
	return 1;
}


class WordCounter
{
public:
	std::unordered_map<int, char*> mWord;
	std::unordered_map<int, int> mFlags, mHits, mHash;
	std::unordered_map<prevkeys, std::unordered_map<int, int>> mLongHit;
	char *mSentence[1024];	
	prevkeys mPrevKeys;
	int mPrevToken;
	int mTokens;
	int mWordno;
	int mSentences;

	int calcHash(char *aString)
	{
		unsigned int i = 0;
		while (*aString)
		{
			i = (i << 11) | (i >> 21);
			i ^= *aString;
	        aString++;
		}    
    
		return i;
	}    

	WordCounter()
	{
		mPrevToken = -1;
		mTokens = 0;
		mWordno = 0;		
		mSentences = 0;
	}

	void tokenRef(int aCurrent)
	{
		prevkeys t = mPrevKeys;
		while (t.depth())
		{
			mLongHit[t][aCurrent]++;
			t.sink();
		}

		mPrevKeys.shift();
		mPrevKeys.a = aCurrent;

		if (mWordno == 0)
			mPrevKeys.reset();
	}

	void addWordcountToken(char *aToken, int aFirst, int aLast)
	{
		int h = calcHash(aToken);
		int i;
		if (aToken == NULL || *aToken == 0)
			return;

		for (i = 0; i < mTokens; i++)
		{
			if (mHash[i] == h && strcmp(mWord[i], aToken) == 0)
			{
				tokenRef(i);
				mHits[i]++;
				mFlags[i] |= (aLast) ? 2 : (aFirst) ? 1 : 0;
				return;
			}
		}

		mWord[mTokens] = _strdup(aToken);
		mHash[mTokens] = h;
		mHits[mTokens] = 1;
		mFlags[mTokens] = (aLast) ? 2 : (aFirst) ? 1 : 0;
		tokenRef(mTokens);
		mTokens++;
	}

	int valid(char *aToken, int &aEnd)
	{
		int allupper = 1;
		int len = 0;
		while (*aToken)
		{
			if ((*aToken >= 'a' && *aToken <= 'z') ||
				(*aToken >= 'A' && *aToken <= 'Z') ||
				*aToken == '\'' ||
				*aToken == ':' ||
				*aToken == ';' ||
				*aToken == '.' ||
				*aToken == '?' ||
				*aToken == '!')
			{
				// Probably valid.
				if ((*aToken == ':' ||
					*aToken == ';' ||
					*aToken == '.' ||
					*aToken == '?' ||
					*aToken == '!'))
				{
					if (*(aToken + 1) != 0)
					{
						return 0; // not valid if not at the end of a string.
					}
					else
					{
						if (mWordno == 0)
						{
							return 0; // and not valid if the only word in the sentence
						}
						if (allupper && len > 1)
						{
							return 0; // all caps, skip
						}
						if (len < 2 && !is_alpha(aToken - 1))
						{
							return 0;
						}

						aEnd = 1; // otherwise valid, and ends the sentence
                        return 1;
                    }                
                }
                if (*aToken >= 'a' && *aToken <= 'z')
                    allupper = 0;
	        }
	        else
	        {
	            return 0;
	        }	                
	        aToken++;
	        len++;
	    }
	    if (allupper && len > 1)
	        return 0;
		if (len < 2 && !is_alpha(aToken - 1))
			return 0;
	    return 1;
	}

	void discardSentence()
	{
		int i;
		for (i = 0; i < mWordno; i++)
			free(mSentence[i]);
		mWordno = 0;
	}

	void flushSentence()
	{
		int i;
		// Make sure sentences start with an upper case character
		if (toupper(mSentence[0][0]) == mSentence[0][0])
		{
			for (i = 0; i < mWordno; i++)
				addWordcountToken(mSentence[i], i == 0, i == (mWordno - 1));
			mSentences++;
		}
		discardSentence();
	}

	void checkWord(char *aString)
	{
		int end = 0;
		if (valid(aString, end))
		{
			mSentence[mWordno] = _strdup(aString);
			mWordno++;
			if (end)
				flushSentence();
		}
		else
		{
			discardSentence();
		}
	}
	
	void wordCount(char *aString)
	{
		char temp[256];
		int p = 0;
		while (*aString)
		{
			temp[p] = *aString;
			if (*aString == '_' || 
			    *aString == '"' || 
			    *aString == ' ' || 
			    *aString == 0 || 
			    *aString < ' ' ||
				*aString > 'z')
			{ 
				temp[p] = 0;
				if (p > 0)
				{
					checkWord(temp);
				}
				p = 0;
			}
			else
			{
				p++;
			}
			aString++;
		}
		temp[p] = 0;
		checkWord(temp);
	}
};

WordCounter gWordCounter;




void read_raw_line(char *aBuffer, FILE * aFile)
{
    int i = 0, count = 0;
    int c;
    do
    {
        c = fgetc(aFile);
        if (c == '\r')
            c = fgetc(aFile);
        if (c == '\t')
            c = ' ';
        if (feof(aFile))
            c = '\n';
        aBuffer[i] = c;
        if (!feof(aFile) && c != '\n')
            i++;
		count++;
    }
    while (!feof(aFile) && c > 31 && !(count > 1000 && c == 32));
    
    // trim trailing spaces:
    while (i >= 0 && whitespace(aBuffer[i])) i--;
    i++;
    
    // terminate
    aBuffer[i] = 0;
}

// Skips empty and commented lines
void read_line(char *aBuffer, FILE * aFile)
{
    do
    {
        read_raw_line(aBuffer, aFile);
    }
    while (!feof(aFile) && aBuffer[0] == '#' && aBuffer[0] > 31);
}

void token(int aTokenIndex, char *aSource, char *aDestination)
{    
    while (aTokenIndex && *aSource)
    {
        while (*aSource && !whitespace(*aSource)) aSource++;
        while (*aSource && whitespace(*aSource)) aSource++;
        aTokenIndex--;
    }
    while (*aSource && !whitespace(*aSource))
    {
        *aDestination = *aSource;
        aDestination++;
        aSource++;
    }
    *aDestination = 0;
}

int tokencmp_for_qsort(const void * a, const void * b)
{
	int idx1 = *(int*)a;
	int idx2 = *(int*)b;
	return gWordCounter.mHits[idx2] - gWordCounter.mHits[idx1];
}

int findflag(int val, int flag)
{
    int i;
    int last = 0;
    for (i = 0; i < gWordCounter.mTokens; i++)
    {
        if ((gWordCounter.mFlags[i] & flag) != 0)
        {
            last = i;
            val -= gWordCounter.mHits[i];
            if (val <= 0)
                return i;
        }
    }
    return last;
}

int findstarter(int val)
{
	return findflag(val, 1);
}

int findender(int val)
{
	return findflag(val, 2);
}

int countflag(int flag)
{
	int i, ssum;
	ssum = 0;
	for (i = 0; i < gWordCounter.mTokens; i++)
	{
		if ((gWordCounter.mFlags[i] & flag) != 0) { ssum += gWordCounter.mHits[i]; }
	}
	return ssum;
}

int countinstflag(int flag)
{
	int i, ssum;
	ssum = 0;
	for (i = 0; i < gWordCounter.mTokens; i++)
	{
		if ((gWordCounter.mFlags[i] & flag) != 0) { ssum++; }
	}
	return ssum;
}

prevkeys gPrevKeys;
int gWordNumber;

//#define DEBUGPRINT

#define SENTENCE_END_WEIGHT ((gWordNumber * gWordNumber) - 100)
// 
#define DEEP_LINK_WEIGHT it->second * t.depth() * t.depth() * t.depth() * t.depth();
// 1, 16, 81, 256, 625, 1296

int nextword(int w)
{
	int sum = 0, n = 0;
	gPrevKeys.shift();
	gPrevKeys.a = w;
	prevkeys t = gPrevKeys;

#ifdef DEBUGPRINT
	printf("[");
#endif

	while (t.depth())
	{
		int n = 0;
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			n++;
			sum += DEEP_LINK_WEIGHT;
			if (gWordCounter.mFlags[it->second] & 2) sum += SENTENCE_END_WEIGHT;
		}
#ifdef DEBUGPRINT
		printf("%c=%d,",
			"?abcdef"[t.depth()], n);
#endif
		t.sink();
	}

	if (sum == 0)
	{
#ifdef DEBUGPRINT
		printf("?]");
#endif
		return findender(rand() % countflag(2));
	}
	sum = rand() % sum;
	int latest;
	t = gPrevKeys;
	while (t.depth())
	{
		for (auto it = gWordCounter.mLongHit[t].begin(); it != gWordCounter.mLongHit[t].end(); ++it)
		{
			sum -= DEEP_LINK_WEIGHT;;
			if (gWordCounter.mFlags[it->second] & 2) sum -= gWordNumber * gWordNumber / 10 - 100;
			if (sum <= 0)
			{
#ifdef DEBUGPRINT
				printf("%c]", "?abcdef"[t.depth()]);
#endif
				return it->first;
			}
			latest = it->first;
		}
		t.sink();
	}
#ifdef DEBUGPRINT
	printf("?]");
#endif
	return latest;
}

int main(int parc, char**pars)
{
    if (parc < 2)
    {
        printf("Gimme a txt file\n");
        return 0;
    }
    
    FILE * f = fopen(pars[1], "r");
    char scratch[8192];
    
	if (!f)
	{
		printf("File not found\n");
		return 0;
	}

	int lines = 0;

	printf("Processing %s\n", pars[1]);

    while (!feof(f))
    {
		lines++;
		if ((lines & 511) == 0 && parc > 2)
			printf("%d lines, %d tokens, %d sentences\r", lines, gWordCounter.mTokens, gWordCounter.mSentences);
        read_line(scratch, f);
        gWordCounter.wordCount(scratch);
    }

        
    fclose(f);
    
    int *idx = new int[gWordCounter.mTokens];
    int i;
	for (i = 0; i < gWordCounter.mTokens; i++)
        idx[i] = i;
        
	qsort(idx, gWordCounter.mTokens, sizeof(int), tokencmp_for_qsort);
	printf("\n\n");
	printf("%d tokens total, %d starters, %d enders, %d sentences\n", gWordCounter.mTokens, countinstflag(1), countinstflag(2), gWordCounter.mSentences);
	printf("Most frequent words in source material:\n");
	int total = gWordCounter.mTokens;
	if (total > 99) total = 99;
	total /= 3;
    for (i = 0; i < total; i++)
    {
        printf("%2d. %-10s(%5d) %2d. %-10s(%5d) %2d. %-10s(%5d)\n",
			i+1, gWordCounter.mWord[idx[i]], gWordCounter.mHits[idx[i]],
			i+1+total, gWordCounter.mWord[idx[i+total]], gWordCounter.mHits[idx[i+total]],
			i+1+total*2, gWordCounter.mWord[idx[i+total*2]], gWordCounter.mHits[idx[i+total*2]]
			);
    }
	printf("\n");

    int s = 0, ssum = 0;

	ssum = countflag(1);

       
    for (i = 0; i < 250; i++)
    {
        s = findstarter(rand() % ssum);
		gPrevKeys.reset();
        
        int ps = -1;
		gWordNumber = 0;
        while ((gWordCounter.mFlags[s] & 2) == 0 && ps != s && gWordNumber < 500)
        {
            printf("%s ", gWordCounter.mWord[s]);
            ps = s;
            s = nextword(s);            
			gWordNumber++;
        }
        printf("%s ", gWordCounter.mWord[s]);
        printf("\n\n");
        
    }
    
    return 0;
}
